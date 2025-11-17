#!/usr/bin/env python3
"""
Audio real-time monitoring and drawing system main program
Based on Qt GUI + Matplotlib + UDP reception + AFSK decoding string
"""

import sys
import asyncio
from graphic import main

if __name__ == '__main__':
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Program interrupted by user")
    except Exception as e:
        print(f"Program execution error: {e}")
        sys.exit(1)
