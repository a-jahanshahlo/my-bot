"""
Real-time AFSK demodulator -based on Goertzel algorithm
"""

import numpy as np
from collections import deque


class TraceGoertzel:
    """Real-time goertzel algorithm implementation"""
    
    def __init__(self, freq: float, n: int):
        """
        Initialize goertzel algorithm
        
        Args:
            freq: Normalized frequency (target frequency/sampling frequency)
            n: window size
        """
        self.freq = freq
        self.n = n
        
        # Precomputed coefficients -consistent with reference code
        self.k = int(freq * n)
        self.w = 2.0 * np.pi * freq
        self.cw = np.cos(self.w)
        self.sw = np.sin(self.w)
        self.c = 2.0 * self.cw
        
        # Initialize state variables -use deque to store the last two values
        self.zs = deque([0.0, 0.0], maxlen=2)
    
    def reset(self):
        """Reset algorithm state"""
        self.zs.clear()
        self.zs.extend([0.0, 0.0])
    
    def __call__(self, xs):
        """
        Processing a set of sample points -Interface consistent with reference code
        
        Args:
            xs: sampling point sequence
            
        Returns:
            Calculated amplitude
        """
        self.reset()
        for x in xs:
            z1, z2 = self.zs[-1], self.zs[-2]  # Z[-1], Z[-2]
            z0 = x + self.c * z1 - z2  # S[n] = x[n] + C * S[n-1] - S[n-2]
            self.zs.append(float(z0))  # update sequence
        return self.amp
    
    @property
    def amp(self) -> float:
        """Calculate current amplitude -consistent with reference code"""
        z1, z2 = self.zs[-1], self.zs[-2]
        ip = self.cw * z1 - z2
        qp = self.sw * z1
        return np.sqrt(ip**2 + qp**2) / (self.n / 2.0)


class PairGoertzel:
    """Dual band goertzel demodulator"""
    
    def __init__(self, f_sample: int, f_space: int, f_mark: int, 
                 bit_rate: int, win_size: int):
        """
     Initialize dual-band demodulator
        
        Args:
            f_sample: sampling frequency
            f_space: Space frequency (usually corresponds to 0)
            f_mark: Mark frequency (usually corresponds to 1)
            bit_rate: bit rate
            win_size: Goertzel window size
        """
        assert f_sample % bit_rate == 0, "The sampling frequency must be an integer multiple of the bit rate"
        
        self.Fs = f_sample
        self.F0 = f_space
        self.F1 = f_mark
        self.bit_rate = bit_rate
        self.n_per_bit = int(f_sample // bit_rate)  # Number of samples per bit
        
        # Calculate normalized frequency
        f1 = f_mark / f_sample
        f0 = f_space / f_sample
        
        # Initialize goertzel algorithm
        self.g0 = TraceGoertzel(freq=f0, n=win_size)
        self.g1 = TraceGoertzel(freq=f1, n=win_size)
        
        #Input buffer
        self.in_buffer = deque(maxlen=win_size)
        self.out_count = 0
        
        print(f"PairGoertzel initialized: f0={f0:.6f}, f1={f1:.6f}, win_size={win_size}, n_per_bit={self.n_per_bit}")
    
    def __call__(self, s: float):
        """
        Processing of single sample points -Interface consistent with reference code
        
        Args:
            s: sampling point value
            
        Returns:
            (amp0, amp1, p1_prob) -spatial frequency amplitude, marker frequency amplitude, marker probability
        """
        self.in_buffer.append(s)
        self.out_count += 1
        
        amp0, amp1, p1_prob = 0, 0, None
        
        # Output the result once per bit period
        if self.out_count >= self.n_per_bit:
            amp0 = self.g0(self.in_buffer)  # Calculate space frequency amplitude
            amp1 = self.g1(self.in_buffer)  # Calculate mark frequency amplitude
            p1_prob = amp1 / (amp0 + amp1 + 1e-8)  # Calculate mark probability
            self.out_count = 0
            
        return amp0, amp1, p1_prob


class RealTimeAFSKDecoder:
    """Real-time AFSK decoder -trigger based on start frame"""
    
    def __init__(self, f_sample: int = 16000, mark_freq: int = 1800, 
                 space_freq: int = 1500, bitrate: int = 100, 
                 s_goertzel: int = 9, threshold: float = 0.5):
        """
        Initialize real-time AFSK decoder
        
        Args:
            f_sample: sampling frequency
            mark_freq: Mark frequency
            space_freq: Space frequency 
            bitrate: bitrate
            s_goertzel: Goertzel window size coefficient (win_size = f_sample //mark_freq *s_goertzel)
            threshold: decision threshold
        """
        self.f_sample = f_sample
        self.mark_freq = mark_freq
        self.space_freq = space_freq
        self.bitrate = bitrate
        self.threshold = threshold
        
        # Calculate window size -same as reference code
        win_size = int(f_sample / mark_freq * s_goertzel)
        
        #Initialize the demodulator
        self.demodulator = PairGoertzel(f_sample, space_freq, mark_freq, 
                                       bitrate, win_size)
        
        # Frame definition -consistent with reference code
        self.start_bytes = b'\x01\x02'
        self.end_bytes = b'\x03\x04'
        self.start_bits = "".join(format(int(x), '08b') for x in self.start_bytes)
        self.end_bits = "".join(format(int(x), '08b') for x in self.end_bytes)

        # State machine
        self.state = "Idle" # idle / entering
        
        # Store demodulation results
        self.buffer_prelude:deque = deque(maxlen=len(self.start_bits)) # Determine whether to start
        self.indicators = []  # Store probability sequence
        self.signal_bits = ""  # store bit sequence
        self.text_cache = ""
        
        #Decoding results
        self.decoded_messages = []
        self.total_bits_received = 0
        
        print(f"Decoder initialized: win_size={win_size}")
        print(f"Start frame: {self.start_bits} (from {self.start_bytes.hex()})")
        print(f"End frame: {self.end_bits} (from {self.end_bytes.hex()})")
    
    def process_audio(self, samples: np.array) -> str:
        """
        Process audio data and return decoded text
        
        Args:
            audio_data: audio byte data (16-bit PCM)
            
        Returns:
            Newly decoded text
        """       
        new_text = "" 
        # Process sampling points one by one
        for sample in samples:
            amp0, amp1, p1_prob = self.demodulator(sample)
            # If there is a probability output, record and judge
            if p1_prob is not None:
                bit = '1' if p1_prob > self.threshold else '0'
                match self.state:
                    case "Idle":
                        self.buffer_prelude.append(bit)
                        pass
                    case "Entering":
                        self.buffer_prelude.append(bit)
                        self.signal_bits += bit
                        self.total_bits_received += 1
                    case _:
                        pass
                self.indicators.append(p1_prob)

                # Check state machine
                if self.state == "Idle" and "".join(self.buffer_prelude) == self.start_bits:
                    self.state = "Entering"
                    self.text_cache = ""
                    self.signal_bits = ""  # Clear the bit sequence
                    self.buffer_prelude.clear()
                elif self.state == "Entering" and ("".join(self.buffer_prelude) == self.end_bits or len(self.signal_bits) >= 256):
                    self.state = "Idle"
                    self.buffer_prelude.clear()

                # Try decoding after collecting a certain number of bits
                if len(self.signal_bits) >= 8:
                    text = self._decode_bits_to_text(self.signal_bits)
                    if len(text) > len(self.text_cache):
                        new_text = text[len(self.text_cache) - len(text):]
                        self.text_cache = text
        return new_text
    
    def _decode_bits_to_text(self, bits: str) -> str:
        """
        Decode bit string to text
        
        Args:
            bits: bit string
            
        Returns:
            decoded text
        """
        if len(bits) < 8:
            return ""
        
        decoded_text = ""
        byte_count = len(bits) // 8
        
        for i in range(byte_count):
            #Extract 8 bits
            byte_bits = bits[i*8:(i+1)*8]
            
            # Convert bits to bytes
            byte_val = int(byte_bits, 2)
            
            # Attempt to decode to ASCII characters
            if 32 <= byte_val <= 126:  # Printable ASCII characters
                decoded_text += chr(byte_val)
            elif byte_val == 0:  # NULL character, ignored
                continue
            else:
                # Non-printable character pass, displayed in hexadecimal
                pass
                # decoded_text += f"\\x{byte_val:02X}"
        
        return decoded_text
    
    def clear(self):
        """Clear decoding status"""
        self.indicators = []
        self.signal_bits = ""
        self.decoded_messages = []
        self.total_bits_received = 0
        print("Decoder status cleared")
    
    def get_stats(self) -> dict:
        """Get decoding statistics"""
        return {
            'prelude_bits': "".join(self.buffer_prelude),
            "state": self.state,
            'total_chars': sum(len(msg) for msg in self.text_cache),
            'buffer_bits': len(self.signal_bits),
            'mark_freq': self.mark_freq,
            'space_freq': self.space_freq,
            'bitrate': self.bitrate,
            'threshold': self.threshold,
        }
