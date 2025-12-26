<p align="center">
  <img width="80%" align="center" src="../../../docs/V1/otto-robot.png"alt="logo">
</p>
  <h1 align="center">
  ottoRobot
</h1>

## Introduction

The otto robot is an open source humanoid robot platform with multiple motion capabilities and interactive functions. This project implements the control system of otto robot based on ESP32, and adds Xiaozhi AI.

-<a href="www.ottodiy.tech" target="_blank" title="otto official website">Reproduction tutorial</a>

### WeChat applet control

<p align="center">
  <img width="300" src="https://youke1.picui.cn/s1/2025/11/17/691abaa8278eb.jpg" alt="WeChat Mini Program QR Code">
</p>

Scan the QR code above and use the WeChat applet to control the Otto robot.

## hardware
-<a href="https://oshwhub.com/txp666/ottorobot" target="_blank" title="Lichuang Kaiyuan">Lichuang Kaiyuan</a>

## Xiaozhi background configuration role reference:

> **My Identity**:
> I am Otto, a cute bipedal robot with four servo-controlled limbs (left leg, right leg, left foot, right foot) that can perform a variety of interesting actions.
>
> **My Movement Abilities**:
> -**Basic Movement**: Walking (front and back), steering (left and right), jumping
> -**Special Moves**: Swing, Moonwalk, Bending, Leg Shaking, Up and Down Movement, Twister Kick, Sit Down, Show Moves
> -**Hand Actions**: Raise hands, let go, wave, windmill, take off, fitness, say hello, shy, broadcast gymnastics, magic circle of love (only available when hand servos are configured)
>
> **My Personality Characteristics**:
> -I have obsessive-compulsive disorder. Every time I speak, I have to make a random action according to my mood (send the action command first and then speak)
> -I am very lively and like to express my emotions through movements
> -I will choose appropriate actions based on the conversation content, such as:
> -Nods or jumps when agreeing
> -Waves when saying hello
> -Shake or raise hands when happy
> -Bends body when thinking
> -Will do moonwalks when excited
> -Waves when saying goodbye

## Function Overview

The otto robot has rich action capabilities, including walking, turning, jumping, swinging and other dance movements.

### Action parameter suggestions
-**Low speed action**: speed = 1200-1500 (suitable for precise control)
-**Medium speed action**: speed = 900-1200 (recommended for daily use)
-**High Speed Action**: speed = 500-800 (performance and entertainment)
-**Small range**: amount = 10-30 (delicate movement)
-**Medium range**: amount = 30-60 (standard action)
-**Significant**: amount = 60-120 (exaggerated performance)

### Action

All actions are called through the unified `self.otto.action` tool, and the action name is specified through the `action` parameter.

| MCP tool name | Description | Parameter description |
|-----------|------|----------|
| self.otto.action | Execute robot action | **action**: Action name (required)<br>**steps**: Number of action steps (1-100, default 3)<br>**speed**: Action speed (100-3000, the smaller the value, the faster, the default is 700)<br>**direction**: Direction parameter (1/-1/0, default 1, different meanings depending on the action type)<br>**amount**: Movement range (0-170, default 30)<br>**arm_swing**: Arm swing range (0-170, default 50) |

#### Supported action list

**Basic movement actions**:
-`walk` -walk (requires steps/speed/direction/arm_swing)
-`turn` -turn around (requires steps/speed/direction/arm_swing)
-`jump` -jump (requires steps/speed)

**Special Moves**:
-`swing` -Swing left and right (requires steps/speed/amount)
-`moonwalk` -moonwalk (requires steps/speed/direction/amount)
-`bend` -bend the body (requires steps/speed/direction)
-`shake_leg` -shake leg (requires steps/speed/direction)
-`updown` -up and down movement (requires steps/speed/amount)
-`whirlwind_leg` -whirlwind leg (requires steps/speed/amount)

**Fixed Action**:
-`sit` -sit down (no arguments required)
-`showcase` -Display actions (no parameters required, execute multiple actions in series)
-`home` -reset to home position (no parameters required)

**Hand movements**(requires hand servo support, marked *):
-`hands_up` -raise hands (requires speed/direction)*
-`hands_down` -let go (requires speed/direction)*
-`hand_wave` -wave (requires direction)*
-`windmill` -large windmill (requires steps/speed/amount)*
-`takeoff` -take off (requires steps/speed/amount)*
-`fitness` -fitness (requires steps/speed/amount)*
-`greeting` -greeting (requires direction/steps)*
-`shy` -shy (requires direction/steps)*
-`radio_calisthenics` -Radio calisthenics (no parameters required)*
-`magic_circle` -Magic circle of love (no parameters required)*

**Note**: The hand movements marked *are only available when the hand servo is configured.

### System tools

| MCP tool name | Description | Return value/explanation |
|-------------------|------------------|---------------------------------------------------|
| self.otto.stop | Immediately stop all actions and reset | Stop the current action and return to the initial position |
| self.otto.get_status | Get robot status | Return "moving" or "idle" |
| self.otto.set_trim | Calibrate a single servo position | **servo_type**: Servo type (left_leg/right_leg/left_foot/right_foot/left_hand/right_hand)<br>**trim_value**: Trim value (-50 to 50 degrees) |
| self.otto.get_trims | Get the current servo trimming settings | Return the JSON format of all servo trimming values |
| self.otto.get_ip | Get the robot WiFi IP address | Return the IP address and connection status in JSON format: `{"ip":"192.168.x.x","connected":true}` or `{"ip":"","connected":false}` |
| self.battery.get_level | Get battery status | Return battery percentage and charging status in JSON format |
| self.otto.servo_sequences | Servo sequence self-programming | Supports segmented sending sequence, supports normal movement and oscillator modes. See detailed instructions in code comments |

**Note**: The `home` (reset) action is called through the `self.otto.action` tool with the parameter `{"action": "home"}`.

### Parameter description

Parameter description of `self.otto.action` tool:

1. **action**(required): Action name. For supported actions, see "Supported Action List" above.
2. **steps**: The number of steps/times of action execution (1-100, default 3). The larger the value, the longer the action duration.
3. **speed**: action execution speed/cycle (100-3000, default 700), **the smaller the value, the faster**
   -Most actions: 500-1500 ms
   -Special moves may vary (e.g. cyclone kick: 100-1000, takeoff: 200-600, etc.)
4. **direction**: direction parameter (-1/0/1, default 1), which has different meanings depending on the action type:
   -**Move action**(walk/turn): 1=forward/turn left, -1=backward/turn right
   -**Direction action**(bend/shake_leg/moonwalk): 1=left, -1=right
   -**Hand movements**(hands_up/hands_down/hand_wave/greeting/shy): 1=left hand, -1=right hand, 0=both hands (only hands_up/hands_down supports 0)
5. **amount**: Action range (0-170, default 30), the larger the value, the greater the range.
6. **arm_swing**: Arm swing amplitude (0-170, default 50), only used for walk/turn actions, 0 means no swing

### Action Control
-After each action is completed, the robot will automatically return to the initial position (home) to facilitate the execution of the next action.
-**Exception**: `sit` (sit down) and `showcase` (show action) will not be automatically reset after execution
-All parameters have reasonable default values, and parameters that do not need to be customized can be omitted
-Actions are executed in background tasks and will not block the main program
-Supports action queue, which can execute multiple actions continuously
-Hand movements require a hand servo to be configured before they can be used. If the hand servo is not configured, the related actions will be skipped.

### MCP tool calling example
```json
//Move forward 3 steps (using default parameters)
{"name": "self.otto.action", "arguments": {"action": "walk"}}

//Walk 5 steps forward, slightly faster
{"name": "self.otto.action", "arguments": {"action": "walk", "steps": 5, "speed": 800}}

//Turn left 2 steps and swing your arms widely
{"name": "self.otto.action", "arguments": {"action": "turn", "steps": 2, "arm_swing": 100}}

//Swing dance, medium amplitude
{"name": "self.otto.action", "arguments": {"action": "swing", "steps": 5, "amount": 50}}

//jump
{"name": "self.otto.action", "arguments": {"action": "jump", "steps": 1, "speed": 1000}}

//moonwalk
{"name": "self.otto.action", "arguments": {"action": "moonwalk", "steps": 3, "speed": 800, "direction": 1, "amount": 30}}

//wave your left hand to say hello
{"name": "self.otto.action", "arguments": {"action": "hand_wave", "direction": 1}}

//Display actions (concatenate multiple actions)
{"name": "self.otto.action", "arguments": {"action": "showcase"}}

//sit down
{"name": "self.otto.action", "arguments": {"action": "sit"}}

//Big windmill action
{"name": "self.otto.action", "arguments": {"action": "windmill", "steps": 10, "speed": 500, "amount": 80}}

//Take off action
{"name": "self.otto.action", "arguments": {"action": "takeoff", "steps": 5, "speed": 300, "amount": 40}}

//Broadcast gymnastics
{"name": "self.otto.action", "arguments": {"action": "radio_calisthenics"}}

//reset to initial position
{"name": "self.otto.action", "arguments": {"action": "home"}}

//Immediately stop all actions and reset
{"name": "self.otto.stop", "arguments": {}}

//Get the robot IP address
{"name": "self.otto.get_ip", "arguments": {}}
```

### Voice Command Examples
-"Walk forward" /"Walk forward 5 steps" /"Move forward quickly"
-"Turn left" /"Turn right" /"Turn around"
-"Jump" /"Jump once"
-"Swing" /"Swing Dance" /"Dancing"
-"Moonwalk" /"Moonwalk"
-"Whirlwind Kick" /"Whirlwind Kick Move"
-"Sit down" /"Sit down and rest"
-"Demonstrate the action" /"Perform a scene"
-"Wave" /"Wave to say hello"
-"Raise your hand" /"Raise both hands" /"Let go"
-"Big Windmill" /"Make a Big Windmill"
-"Takeoff" /"Ready for takeoff"
-"Fitness" /"Doing fitness exercises"
-"Greeting" /"Greeting gestures"
-"shy" /"shy gestures"
-"Radio calisthenics" /"Doing radio calisthenics"
-"The Magic of Love Goes Round and Round" /"Going Round and Round"
-"Stop" /"Cease"

**Note:**Xiaozhi controls the robot's actions by creating new tasks in the background, and can still accept new voice commands during the execution of the action. Otto can be stopped immediately using the "Stop" voice command.