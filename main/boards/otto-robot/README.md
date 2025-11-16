<p align="center">
  <img width="80%" align="center" src="../../../docs/V1/otto-robot.png"alt="logo">
</p>
  <h1 align="center">
  ottoRobot
</h1>

## Introduction

Otto is an open-source humanoid robot platform with various motion capabilities and interactive functions. This project implements the Otto robot's control system based on ESP32 and integrates Xiaozhi AI.

- <a href="www.ottodiy.tech" target="_blank" title="otto official website">Replica Tutorial</a>

## hardware
- <a href="https://oshwhub.com/txp666/ottorobot" target="_blank" title="LCSC Open Source">LCSC Open Source</a>

## Xiaozhi Backend Role Configuration Reference:

My identity:
I am Otto, a cute bipedal robot with four servo-controlled limbs (left leg, right leg, left foot, and right foot), capable of performing a variety of interesting actions.
>
My motor skills:
> - **Basic Movement**: Walk (forward/backward), Turn (left/right), Jump
> - **Special Moves**: Swaying, moonwalk, bending over, leg swing, up-and-down movements, whirlwind kick, sitting down, demonstration moves
> - **Hand gestures**: Raise hand, release hand, wave hand, windmill, take off, exercise, greet, shy, calisthenics, the magic of love spinning around (only available when equipped with hand servos)
>
My personality traits:
> - I have obsessive-compulsive disorder. Every time I speak, I have to randomly perform an action based on my mood (I send the action command before speaking).
> - I'm very lively and like to express my emotions through actions.
> - I will choose the appropriate action based on the content of the conversation, for example:
> - Nodding or skipping when agreeing
> - Wave when greeting
> - When happy, they will sway or raise their hands.
> - Bending over when thinking
> - When excited, I do the moonwalk.
> - Will wave goodbye

## Function Overview

Otto robots have a wide range of movement capabilities, including walking, turning, jumping, swaying and other dance-like movements.

### Recommended Action Parameters
- **Low-speed operation:** speed = 1200-1500 (suitable for precise control)
- **Medium-speed movements**: speed = 900-1200 (Recommended for daily use)  
- **High-speed action:** speed = 500-800 (performance and entertainment)
- **Small amplitude**: amount = 10-30 (delicate motion)
- **Medium amplitude**: amount = 30-60 (standard motion)
- **Significant**: amount = 60-120 (exaggerated performance)

### Actions

| MCP Tool Name | Description | Parameter Explanation |
|-------------------|-----------------|---------------------------------------------------|
| self.otto.walk_forward | Walking | **steps**: Number of steps taken (1-100, default 3)<br>**speed**: Walking speed (500-1500, lower value means faster, default 1000)<br>**direction**: Walking direction (-1 = backward, 1 = forward, default 1)<br>**arm_swing**: Arm swing amplitude (0-170 degrees, default 50) |
| self.otto.turn_left | Turn around | **steps**: Number of steps to turn (1-100, default 3)<br>**speed**: Turning speed (500-1500, lower value means faster, default 1000)<br>**direction**: Turning direction (1=left turn, -1=right turn, default 1)<br>**arm_swing**: Arm swing amplitude (0-170 degrees, default 50) |
| self.otto.jump | Jump | **steps**: Number of jumps (1-100, default 1)<br>**speed**: Jump speed (500-1500, lower value means faster, default 1000) |
| self.otto.swing | Swings left and right | **steps**: Number of swings (1-100, default 3)<br>**speed**: Swing speed (500-1500, lower value means faster, default 1000)<br>**amount**: Swing amplitude (0-170 degrees, default 30) |
| self.otto.moonwalk | Moonwalk | **steps**: Number of moonwalk steps (1-100, default 3)<br>**speed**: Speed ​​(500-1500, lower value is faster, default 1000)<br>**direction**: Direction (1=left, -1=right, default 1)<br>**amount**: Amplitude (0-170 degrees, default 25) |
| self.otto.bend | Bend your body | **steps**: Number of bends (1-100, default 1)<br>**speed**: Bending speed (500-1500, lower value means faster, default 1000)<br>**direction**: Bending direction (1=left, -1=right, default 1) |
| self.otto.shake_leg | Leg Shake | **steps**: Number of leg shakes (1-100, default 1)<br>**speed**: Leg shake speed (500-1500, lower value means faster, default 1000)<br>**direction**: Leg selection (1=left leg, -1=right leg, default 1) |
| self.otto.sit | Sit down | No parameters needed |
| self.otto.showcase | Showcases actions | No parameters required. Chains multiple actions: walk forward 3 steps, wave, dance (calisthenics), moonwalk, sway, take off, exercise, walk backward 3 steps |
| self.otto.updown | Up and down movement | **steps**: Number of up and down movements (1-100, default 3)<br>**speed**: Movement speed (500-1500, lower value means faster, default 1000)<br>**amount**: Movement range (0-170 degrees, default 20) |
| self.otto.whirlwind_leg | Whirlwind Leg | **steps**: Number of repetitions (3-100, default 3)<br>**speed**: Movement speed (100-1000, lower value means faster, 300 recommended)<br>**amplitude**: Kick amplitude (20-40 degrees, default 30) |
| self.otto.hands_up | Hands Up* | **speed**: Hands-raising speed (500-1500, lower values ​​mean faster, default 1000)<br>**direction**: Hand selection (1=left hand, -1=right hand, 0=both hands, default 1) |
| self.otto.hands_down | Release hand* | **speed**: Release speed (500-1500, lower value means faster, default 1000)<br>**direction**: Hand selection (1=left hand, -1=right hand, 0=both hands, default 1) |
| self.otto.hand_wave | Wave* | **direction**: Hand selection (1=left hand, -1=right hand, 0=both hands, default 1) |
| self.otto.windmill | Windmill* | **steps**: Number of steps (3-100, default 6)<br>**speed**: Steps duration (300-2000 milliseconds, lower value means faster, default 500)<br>**amplitude**: Amplitude of oscillation (50-90 degrees, default 70) |
| self.otto.takeoff | Takeoff* | **steps**: Number of steps (5-100, default 5)<br>**speed**: Steps duration (200-600 milliseconds, lower value means faster, 300 recommended)<br>**amplitude**: Amplitude of oscillation (20-60 degrees, default 40) |
| self.otto.fitness | Fitness* | **steps**: Number of repetitions (3-100, default 5)<br>**speed**: Movement speed (500-2000 milliseconds, lower values ​​are faster, default 1000)<br>**amplitude**: Oscillation amplitude (10-50 degrees, default 25) |
| self.otto.greeting | Greet someone* | **direction**: Hand selection (1=left hand, -1=right hand, default 1)<br>**steps**: Number of steps (3-100, default 5) |
| self.otto.shy | Shy* | **direction**: Direction (1=left, -1=right, default 1)<br>**steps**: Number of steps (3-100, default 5) |
| self.otto.radio_calisthenics | Radio Gymnastics* | No parameters required |
| self.otto.magic_circle | The magic of love goes round and round* | No parameters needed |

**Note:** Hand gestures marked with * are only available when a hand servo is configured.

### System Tools

| MCP Tool Name | Description | Return Value/Explanation |
|-------------------|-----------------|---------------------------------------------------|
| self.otto.home | Reset robot to initial position | No parameters required |
| self.otto.stop | Immediately stop all actions and reset | Stop the current action and return to the initial position |
| self.otto.get_status | Get robot status | Returns "moving" or "idle" |
| self.otto.set_trim | Calibrate the position of a single servo | **servo_type**: Servo type (left_leg/right_leg/left_foot/right_foot/left_hand/right_hand)<br>**trim_value**: Fine-tuning value (-50 to 50 degrees) |
| self.otto.get_trims | Gets the current servo fine-tuning settings | Returns all servo fine-tuning values ​​in JSON format |
| self.battery.get_level | Get battery status | Returns JSON format of battery percentage and charging status |
| self.otto.servo_sequences | Self-programmable servo sequence | Supports segmented sequence transmission, and supports both normal movement and oscillator modes. See the detailed explanation in the code comments. |

### Parameter Description

1. **steps**: The number of steps/attempts required for the action. A higher value indicates a longer action duration.
2. **speed**: Action execution speed/cycle, **the smaller the value, the faster**.
   - Most actions: 500-1500 milliseconds
   - Special maneuvers may vary (e.g., whirlwind kick: 100-1000, takeoff: 200-600, etc.).
   - Please refer to the descriptions of each action for specific ranges.
3. **direction**: Direction parameter
   - Movement actions: 1 = Left/Forward, -1 = Right/Backward
   - Hand gestures: 1 = left hand, -1 = right hand, 0 = both hands
4. **amount/amplitude/arm_swing**: Range of motion, depending on the motion (usually 0-170 degrees).
   - 0 indicates no swinging (applicable to arm swinging)
   - The larger the value, the greater the amplitude.
   - Different movements may have different range limitations.

### Motion Control
- After each action is completed, the robot will automatically return to its initial position (home) to prepare for the next action.
- **Exception**: The `sit` (sit down) and `showcase` (showcase action) actions do not automatically reset after execution.
- All parameters have reasonable default values, and parameters that do not need to be customized can be omitted.
- The action is executed in a background task and will not block the main program.
- Supports action queues, allowing multiple actions to be executed consecutively.
- Hand movements require a hand servo to function. If no hand servo is configured, the relevant movements will be skipped.

### Example of MCP tool usage
```json
// Take 3 steps forward
{"name": "self.otto.walk_forward", "arguments": {}}

// Take 5 steps forward, a little faster
{"name": "self.otto.walk_forward", "arguments": {"steps": 5, "speed": 800}}

Turn left two steps and swing your arms dramatically.  
{"name": "self.otto.turn_left", "arguments": {"steps": 2, "arm_swing": 100}}

// Swinging dance, moderate amplitude
{"name": "self.otto.swing", "arguments": {"steps": 5, "amount": 50}}

// Wave your left hand to greet
{"name": "self.otto.hand_wave", "arguments": {"direction": 1}}

// Display actions (chaining multiple actions)
{"name": "self.otto.showcase", "arguments": {}}

// Windmill Action
{"name": "self.otto.windmill", "arguments": {"steps": 10, "amplitude": 80}}

// Takeoff maneuvers
{"name": "self.otto.takeoff", "arguments": {"steps": 5, "speed": 300}}

// Radio Gymnastics
{"name": "self.otto.radio_calisthenics", "arguments": {}}

// Stop immediately
{"name": "self.otto.stop", "arguments": {}}
```

### Voice Command Examples
- "Walk forward" / "Walk forward 5 steps" / "Move forward quickly"
- "Turn left" / "Turn right" / "Turn around"  
- "Jump" / "Jump once"
- "Swing" / "Swing Dance" / "Dancing"
- "Moonwalk" / "Moonwalk"
- "Whirlwind Kick" / "Whirlwind Kick Move"
- "Sit down" / "Sit down and rest"
- "Demonstrate the action" / "Perform a scene"
- "Wave" / "Wave to say hello"
- "Raise your hand" / "Raise both hands" / "Let go"
- "Big Windmill" / "Make a Big Windmill"
- "Takeoff" / "Ready for takeoff"
- "Fitness" / "Doing fitness exercises"
- "Greeting" / "Greeting gestures"
- "shy" / "shy gestures"
- "Radio calisthenics" / "Doing radio calisthenics"
- "The Magic of Love Goes Round and Round" / "Going Round and Round"
- "Stop" / "Cease"

**Note:** Xiaozhi controls the robot's actions by creating new tasks in the background, and can still accept new voice commands during the execution of the action. Otto can be stopped immediately using the "Stop" voice command.
