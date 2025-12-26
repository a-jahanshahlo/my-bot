/*
    Otto Robot Controller -MCP Protocol Version
*/

#include <cJSON.h>
#include <esp_log.h>

#include <cstdlib>
#include <cstring>

#include "application.h"
#include "board.h"
#include "config.h"
#include "mcp_server.h"
#include "otto_movements.h"
#include "power_manager.h"
#include "sdkconfig.h"
#include "settings.h"
#include <wifi_manager.h>

#define TAG "OttoController"

class OttoController
{
private:
    Otto otto_;
    TaskHandle_t action_task_handle_ = nullptr;
    QueueHandle_t action_queue_;
    bool has_hands_ = false;
    bool is_action_in_progress_ = false;

    struct OttoActionParams
    {
        int action_type;
        int steps;
        int speed;
        int direction;
        int amount;
        char servo_sequence_json[512]; // json string used to store servo sequence
    };

    enum ActionType
    {
        ACTION_WALK = 1,
        ACTION_TURN = 2,
        ACTION_JUMP = 3,
        ACTION_SWING = 4,
        ACTION_MOONWALK = 5,
        ACTION_BEND = 6,
        ACTION_SHAKE_LEG = 7,
        ACTION_SIT = 25, // sit down

        ACTION_RADIO_CALISTHENICS = 26, // radio gymnastics

        ACTION_MAGIC_CIRCLE = 27, // The magic of love goes round and round

        ACTION_UPDOWN = 8,
        ACTION_TIPTOE_SWING = 9,
        ACTION_JITTER = 10,
        ACTION_ASCENDING_TURN = 11,
        ACTION_CRUSAITO = 12,
        ACTION_FLAPPING = 13,
        ACTION_HANDS_UP = 14,
        ACTION_HANDS_DOWN = 15,
        ACTION_HAND_WAVE = 16,
        ACTION_WINDMILL = 20, // Big windmill

        ACTION_TAKEOFF = 21, // take off

        ACTION_FITNESS = 22, // fitness

        ACTION_GREETING = 23, // greet

        ACTION_SHY = 24, // shy

        ACTION_SHOWCASE = 28, // show action

        ACTION_HOME = 17,
        ACTION_SERVO_SEQUENCE = 18, // Servo sequence (self-programming)

        ACTION_WHIRLWIND_LEG = 19 // Tornado Kick

    };

    static void ActionTask(void *arg)
    {
        OttoController *controller = static_cast<OttoController *>(arg);
        OttoActionParams params;
        controller->otto_.AttachServos();

        while (true)
        {
            if (xQueueReceive(controller->action_queue_, &params, pdMS_TO_TICKS(1000)) == pdTRUE)
            {
                ESP_LOGI(TAG, "执行动作: %d", params.action_type);
                PowerManager::PauseBatteryUpdate(); // 动作开始时暂停电量更新
                controller->is_action_in_progress_ = true;
                if (params.action_type == ACTION_SERVO_SEQUENCE)
                {
                    // Execute servo sequence (self-programming) -only supports short key name format

                    cJSON *json = cJSON_Parse(params.servo_sequence_json);
                    if (json != nullptr)
                    {
                        ESP_LOGD(TAG, "Json parsed successfully, length=%d", strlen(params.servo_sequence_json));
                        // Use the short key name "a" to represent the action array

                        cJSON *actions = cJSON_GetObjectItem(json, "a");
                        if (cJSON_IsArray(actions))
                        {
                            int array_size = cJSON_GetArraySize(actions);
                            ESP_LOGI(TAG, "Execute servo sequence, total %d actions", array_size);

                            // Get the delay after the sequence execution is completed (short name "d", top-level parameter)

                            int sequence_delay = 0;
                            cJSON *delay_item = cJSON_GetObjectItem(json, "d");
                            if (cJSON_IsNumber(delay_item))
                            {
                                sequence_delay = delay_item->valueint;
                                if (sequence_delay < 0)
                                    sequence_delay = 0;
                            }

                            // Initialize the current servo position (used to maintain unspecified servo positions)

                            int current_positions[SERVO_COUNT];
                            for (int j = 0; j < SERVO_COUNT; j++)
                            {
                                current_positions[j] = 90; // Default middle position
                            }
                            // Hand servo default position

                            current_positions[LEFT_HAND] = 45;
                            current_positions[RIGHT_HAND] = 180 - 45;

                            for (int i = 0; i < array_size; i++)
                            {
                                cJSON *action_item = cJSON_GetArrayItem(actions, i);
                                if (cJSON_IsObject(action_item))
                                {
                                    // Check if it is in oscillator mode (short keyname "osc")

                                    cJSON *osc_item = cJSON_GetObjectItem(action_item, "osc");
                                    if (cJSON_IsObject(osc_item))
                                    {
                                        // Oscillator mode -using Execute2, oscillates centered on absolute angle

                                        int amplitude[SERVO_COUNT] = {0};
                                        int center_angle[SERVO_COUNT] = {0};
                                        double phase_diff[SERVO_COUNT] = {0};
                                        int period = 300; // Default period 300 milliseconds

                                        float steps = 8.0; // Default step count 8.0

                                        const char *servo_names[] = {"ll", "rl", "lf", "rf", "lh", "rh"};

                                        // Read amplitude (short key name "a"), default is 0 degrees

                                        for (int j = 0; j < SERVO_COUNT; j++)
                                        {
                                            amplitude[j] = 0; // Default amplitude 0 degrees
                                        }
                                        cJSON *amp_item = cJSON_GetObjectItem(osc_item, "a");
                                        if (cJSON_IsObject(amp_item))
                                        {
                                            for (int j = 0; j < SERVO_COUNT; j++)
                                            {
                                                cJSON *amp_value = cJSON_GetObjectItem(amp_item, servo_names[j]);
                                                if (cJSON_IsNumber(amp_value))
                                                {
                                                    int amp = amp_value->valueint;
                                                    if (amp >= 10 && amp <= 90)
                                                    {
                                                        amplitude[j] = amp;
                                                    }
                                                }
                                            }
                                        }

                                        // Read the center angle (short key name "o"), default 90 degrees (absolute angle 0-180 degrees)

                                        for (int j = 0; j < SERVO_COUNT; j++)
                                        {
                                            center_angle[j] = 90; // The default center angle is 90 degrees (middle position)
                                        }
                                        cJSON *center_item = cJSON_GetObjectItem(osc_item, "o");
                                        if (cJSON_IsObject(center_item))
                                        {
                                            for (int j = 0; j < SERVO_COUNT; j++)
                                            {
                                                cJSON *center_value = cJSON_GetObjectItem(center_item, servo_names[j]);
                                                if (cJSON_IsNumber(center_value))
                                                {
                                                    int center = center_value->valueint;
                                                    if (center >= 0 && center <= 180)
                                                    {
                                                        center_angle[j] = center;
                                                    }
                                                }
                                            }
                                        }

                                        // Safety check: Prevent left and right legs from oscillating at the same time (amplitude check)

                                        const int LARGE_AMPLITUDE_THRESHOLD = 40; // Large Amplitude Threshold: 40 degrees

                                        bool left_leg_large = amplitude[LEFT_LEG] >= LARGE_AMPLITUDE_THRESHOLD;
                                        bool right_leg_large = amplitude[RIGHT_LEG] >= LARGE_AMPLITUDE_THRESHOLD;
                                        bool left_foot_large = amplitude[LEFT_FOOT] >= LARGE_AMPLITUDE_THRESHOLD;
                                        bool right_foot_large = amplitude[RIGHT_FOOT] >= LARGE_AMPLITUDE_THRESHOLD;

                                        if (left_leg_large && right_leg_large)
                                        {
                                            ESP_LOGW(TAG, "It is detected that the left and right legs oscillate greatly at the same time, and the amplitude of the right leg is limited.");
                                            amplitude[RIGHT_LEG] = 0; // Prohibit right leg oscillation
                                        }
                                        if (left_foot_large && right_foot_large)
                                        {
                                            ESP_LOGW(TAG, "It is detected that the left and right feet oscillate greatly at the same time, and the amplitude of the right foot is limited.");
                                            amplitude[RIGHT_FOOT] = 0; // Prohibit right foot oscillation
                                        }

                                        // Read the phase difference (short name "ph", in degrees, converted to radians)

                                        cJSON *phase_item = cJSON_GetObjectItem(osc_item, "ph");
                                        if (cJSON_IsObject(phase_item))
                                        {
                                            for (int j = 0; j < SERVO_COUNT; j++)
                                            {
                                                cJSON *phase_value = cJSON_GetObjectItem(phase_item, servo_names[j]);
                                                if (cJSON_IsNumber(phase_value))
                                                {
                                                    // Convert degrees to radians

                                                    phase_diff[j] = phase_value->valuedouble * 3.141592653589793 / 180.0;
                                                }
                                            }
                                        }

                                        // Read period (short key name "p"), range 100-3000 milliseconds

                                        cJSON *period_item = cJSON_GetObjectItem(osc_item, "p");
                                        if (cJSON_IsNumber(period_item))
                                        {
                                            period = period_item->valueint;
                                            if (period < 100)
                                                period = 100;
                                            if (period > 3000)
                                                period = 3000; // As described, limited to 3000 milliseconds
                                        }

                                        // Number of read cycles (short key name "c"), range 0.1-20.0

                                        cJSON *steps_item = cJSON_GetObjectItem(osc_item, "c");
                                        if (cJSON_IsNumber(steps_item))
                                        {
                                            steps = (float)steps_item->valuedouble;
                                            if (steps < 0.1)
                                                steps = 0.1;
                                            if (steps > 20.0)
                                                steps = 20.0; // As described, limit 20.0
                                        }

                                        // Execute oscillation -using Execute2, centered on absolute angle

                                        ESP_LOGI(TAG, "perform oscillating action%d: period=%d, steps=%.1f", i, period, steps);
                                        controller->otto_.Execute2(amplitude, center_angle, period, phase_diff, steps);

                                        // Update position after oscillation (use center angle as final position)

                                        for (int j = 0; j < SERVO_COUNT; j++)
                                        {
                                            current_positions[j] = center_angle[j];
                                        }
                                    }
                                    else
                                    {
                                        // Normal movement mode
                                        // Copy from the current position array, keeping the unspecified servo position

                                        int servo_target[SERVO_COUNT];
                                        for (int j = 0; j < SERVO_COUNT; j++)
                                        {
                                            servo_target[j] = current_positions[j];
                                        }

                                        // Read servo position from JSON (short key name "s")

                                        cJSON *servos_item = cJSON_GetObjectItem(action_item, "s");
                                        if (cJSON_IsObject(servos_item))
                                        {
                                            // Short key name: ll/rl/lf/rf/lh/rh

                                            const char *servo_names[] = {"ll", "rl", "lf", "rf", "lh", "rh"};

                                            for (int j = 0; j < SERVO_COUNT; j++)
                                            {
                                                cJSON *servo_value = cJSON_GetObjectItem(servos_item, servo_names[j]);
                                                if (cJSON_IsNumber(servo_value))
                                                {
                                                    int position = servo_value->valueint;
                                                    // Limit the position range to 0 180 degrees

                                                    if (position >= 0 && position <= 180)
                                                    {
                                                        servo_target[j] = position;
                                                    }
                                                }
                                            }
                                        }

                                        // 获取移动速度（短键名 "v"，默认1000毫秒）
                                        int speed = 1000;
                                        cJSON *speed_item = cJSON_GetObjectItem(action_item, "v");
                                        if (cJSON_IsNumber(speed_item))
                                        {
                                            speed = speed_item->valueint;
                                            if (speed < 100)
                                                speed = 100; // Minimum 100 milliseconds

                                            if (speed > 3000)
                                                speed = 3000; // Maximum 3000 milliseconds
                                        }

                                        // Execute servo movement

                                        ESP_LOGI(TAG, "perform action%d: ll=%d, rl=%d, lf=%d, rf=%d, v=%d",
                                                 i, servo_target[LEFT_LEG], servo_target[RIGHT_LEG],
                                                 servo_target[LEFT_FOOT], servo_target[RIGHT_FOOT], speed);
                                        controller->otto_.MoveServos(speed, servo_target);

                                        // Update the current position array for the next action

                                        for (int j = 0; j < SERVO_COUNT; j++)
                                        {
                                            current_positions[j] = servo_target[j];
                                        }
                                    }

                                    // Get the delay time after the action (short key name "d")

                                    int delay_after = 0;
                                    cJSON *delay_item = cJSON_GetObjectItem(action_item, "d");
                                    if (cJSON_IsNumber(delay_item))
                                    {
                                        delay_after = delay_item->valueint;
                                        if (delay_after < 0)
                                            delay_after = 0;
                                    }

                                    // Delay after action (no delay after last action)

                                    if (delay_after > 0 && i < array_size - 1)
                                    {
                                        ESP_LOGI(TAG, "Action %d execution completed, delay %d milliseconds", i, delay_after);
                                        vTaskDelay(pdMS_TO_TICKS(delay_after));
                                    }
                                }
                            }

                            // Delay after sequence execution completes (used for pauses between sequences)

                            if (sequence_delay > 0)
                            {
                                // Check if there are still sequences in the queue to be executed

                                UBaseType_t queue_count = uxQueueMessagesWaiting(controller->action_queue_);
                                if (queue_count > 0)
                                {
                                    ESP_LOGI(TAG, "The sequence execution is completed, and the next sequence is executed after a delay of %d milliseconds (there are still %d sequences in the queue)",
                                             sequence_delay, queue_count);
                                    vTaskDelay(pdMS_TO_TICKS(sequence_delay));
                                }
                            }
                            // Release json memory

                            cJSON_Delete(json);
                        }
                        else
                        {
                            ESP_LOGE(TAG, "Servo sequence format error: 'a' is not an array");
                            cJSON_Delete(json);
                        }
                    }
                    else
                    {
                        // Get the error message of c json

                        const char *error_ptr = cJSON_GetErrorPtr();
                        int json_len = strlen(params.servo_sequence_json);
                        ESP_LOGE(TAG, "Failed to parse servo sequence json, length = %d, error location: %s", json_len,
                                 error_ptr ? error_ptr : "unknown");
                        ESP_LOGE(TAG, "Json content: %s", params.servo_sequence_json);
                    }
                }
                else
                {
                    // Execute predefined actions

                    switch (params.action_type)
                    {
                    case ACTION_WALK:
                        controller->otto_.Walk(params.steps, params.speed, params.direction,
                                               params.amount);
                        break;
                    case ACTION_TURN:
                        controller->otto_.Turn(params.steps, params.speed, params.direction,
                                               params.amount);
                        break;
                    case ACTION_JUMP:
                        controller->otto_.Jump(params.steps, params.speed);
                        break;
                    case ACTION_SWING:
                        controller->otto_.Swing(params.steps, params.speed, params.amount);
                        break;
                    case ACTION_MOONWALK:
                        controller->otto_.Moonwalker(params.steps, params.speed, params.amount,
                                                     params.direction);
                        break;
                    case ACTION_BEND:
                        controller->otto_.Bend(params.steps, params.speed, params.direction);
                        break;
                    case ACTION_SHAKE_LEG:
                        controller->otto_.ShakeLeg(params.steps, params.speed, params.direction);
                        break;
                    case ACTION_SIT:
                        controller->otto_.Sit();
                        break;
                    case ACTION_RADIO_CALISTHENICS:
                        if (controller->has_hands_)
                        {
                            controller->otto_.RadioCalisthenics();
                        }
                        break;
                    case ACTION_MAGIC_CIRCLE:
                        if (controller->has_hands_)
                        {
                            controller->otto_.MagicCircle();
                        }
                        break;
                    case ACTION_SHOWCASE:
                        controller->otto_.Showcase();
                        break;
                    case ACTION_UPDOWN:
                        controller->otto_.UpDown(params.steps, params.speed, params.amount);
                        break;
                    case ACTION_TIPTOE_SWING:
                        controller->otto_.TiptoeSwing(params.steps, params.speed, params.amount);
                        break;
                    case ACTION_JITTER:
                        controller->otto_.Jitter(params.steps, params.speed, params.amount);
                        break;
                    case ACTION_ASCENDING_TURN:
                        controller->otto_.AscendingTurn(params.steps, params.speed, params.amount);
                        break;
                    case ACTION_CRUSAITO:
                        controller->otto_.Crusaito(params.steps, params.speed, params.amount,
                                                   params.direction);
                        break;
                    case ACTION_FLAPPING:
                        controller->otto_.Flapping(params.steps, params.speed, params.amount,
                                                   params.direction);
                        break;
                    case ACTION_WHIRLWIND_LEG:
                        controller->otto_.WhirlwindLeg(params.steps, params.speed, params.amount);
                        break;
                    case ACTION_HANDS_UP:
                        if (controller->has_hands_)
                        {
                            controller->otto_.HandsUp(params.speed, params.direction);
                        }
                        break;
                    case ACTION_HANDS_DOWN:
                        if (controller->has_hands_)
                        {
                            controller->otto_.HandsDown(params.speed, params.direction);
                        }
                        break;
                    case ACTION_HAND_WAVE:
                        if (controller->has_hands_)
                        {
                            controller->otto_.HandWave(params.direction);
                        }
                        break;
                    case ACTION_WINDMILL:
                        if (controller->has_hands_)
                        {
                            controller->otto_.Windmill(params.steps, params.speed, params.amount);
                        }
                        break;
                    case ACTION_TAKEOFF:
                        if (controller->has_hands_)
                        {
                            controller->otto_.Takeoff(params.steps, params.speed, params.amount);
                        }
                        break;
                    case ACTION_FITNESS:
                        if (controller->has_hands_)
                        {
                            controller->otto_.Fitness(params.steps, params.speed, params.amount);
                        }
                        break;
                    case ACTION_GREETING:
                        if (controller->has_hands_)
                        {
                            controller->otto_.Greeting(params.direction, params.steps);
                        }
                        break;
                    case ACTION_SHY:
                        if (controller->has_hands_)
                        {
                            controller->otto_.Shy(params.direction, params.steps);
                        }
                        break;
                    case ACTION_HOME:
                        controller->otto_.Home(true);
                        break;
                    }
                    if (params.action_type != ACTION_SIT)
                    {
                        if (params.action_type != ACTION_HOME && params.action_type != ACTION_SERVO_SEQUENCE)
                        {
                            controller->otto_.Home(params.action_type != ACTION_HANDS_UP);
                        }
                    }
                }
                controller->is_action_in_progress_ = false;
                PowerManager::ResumeBatteryUpdate(); // 动作结束时恢复电量更新
                vTaskDelay(pdMS_TO_TICKS(20));
            }
        }
    }

    void StartActionTaskIfNeeded()
    {
        if (action_task_handle_ == nullptr)
        {
            xTaskCreate(ActionTask, "otto_action", 1024 * 3, this, configMAX_PRIORITIES - 1,
                        &action_task_handle_);
        }
    }

    void QueueAction(int action_type, int steps, int speed, int direction, int amount)
    {
        // Check hand movements

        if ((action_type >= ACTION_HANDS_UP && action_type <= ACTION_HAND_WAVE) ||
            (action_type == ACTION_WINDMILL) || (action_type == ACTION_TAKEOFF) ||
            (action_type == ACTION_FITNESS) || (action_type == ACTION_GREETING) ||
            (action_type == ACTION_SHY) || (action_type == ACTION_RADIO_CALISTHENICS) ||
            (action_type == ACTION_MAGIC_CIRCLE))
        {
            if (!has_hands_)
            {
                ESP_LOGW(TAG, "Trying to perform a hand movement, but the robot is not configured with hand servos");
                return;
            }
        }

        ESP_LOGI(TAG, "Action control: type=%d, number of steps=%d, speed=%d, direction=%d, amplitude=%d", action_type, steps,
                 speed, direction, amount);

        OttoActionParams params = {action_type, steps, speed, direction, amount, ""};
        xQueueSend(action_queue_, &params, portMAX_DELAY);
        StartActionTaskIfNeeded();
    }

    void QueueServoSequence(const char *servo_sequence_json)
    {
        if (servo_sequence_json == nullptr)
        {
            ESP_LOGE(TAG, "Sequence json is empty");
            return;
        }

        int input_len = strlen(servo_sequence_json);
        const int buffer_size = 512; // Servo sequence json array size

        ESP_LOGI(TAG, "Queue servo sequence, input length=%d, buffer size=%d", input_len, buffer_size);

        if (input_len >= buffer_size)
        {
            ESP_LOGE(TAG, "Json string is too long! Input length=%d, maximum allowed=%d", input_len, buffer_size - 1);
            return;
        }

        if (input_len == 0)
        {
            ESP_LOGW(TAG, "Sequence json is empty string");
            return;
        }

        OttoActionParams params = {ACTION_SERVO_SEQUENCE, 0, 0, 0, 0, ""};
        // Copy json string into structure (limited length)

        strncpy(params.servo_sequence_json, servo_sequence_json, sizeof(params.servo_sequence_json) - 1);
        params.servo_sequence_json[sizeof(params.servo_sequence_json) - 1] = '\0';

        ESP_LOGD(TAG, "Sequence has been queued: %s", params.servo_sequence_json);

        xQueueSend(action_queue_, &params, portMAX_DELAY);
        StartActionTaskIfNeeded();
    }

    void LoadTrimsFromNVS()
    {
        Settings settings("otto_trims", false);

        int left_leg = settings.GetInt("left_leg", 0);
        int right_leg = settings.GetInt("right_leg", 0);
        int left_foot = settings.GetInt("left_foot", 0);
        int right_foot = settings.GetInt("right_foot", 0);
        int left_hand = settings.GetInt("left_hand", 0);
        int right_hand = settings.GetInt("right_hand", 0);

        ESP_LOGI(TAG, "Load fine-tuning settings from NVS: left leg=%d, right leg=%d, left foot=%d, right foot=%d, left hand=%d, right hand=%d",
                 left_leg, right_leg, left_foot, right_foot, left_hand, right_hand);

        otto_.SetTrims(left_leg, right_leg, left_foot, right_foot, left_hand, right_hand);
    }

public:
    OttoController(const HardwareConfig &hw_config)
    {
        otto_.Init(
            hw_config.left_leg_pin,
            hw_config.right_leg_pin,
            hw_config.left_foot_pin,
            hw_config.right_foot_pin,
            hw_config.left_hand_pin,
            hw_config.right_hand_pin);

        has_hands_ = (hw_config.left_hand_pin != GPIO_NUM_NC && hw_config.right_hand_pin != GPIO_NUM_NC);
        ESP_LOGI(TAG, "Otto机器人初始化%s手部舵机", has_hands_ ? "带" : "不带");
        ESP_LOGI(TAG, "舵机引脚配置: LL=%d, RL=%d, LF=%d, RF=%d, LH=%d, RH=%d",
                 hw_config.left_leg_pin, hw_config.right_leg_pin,
                 hw_config.left_foot_pin, hw_config.right_foot_pin,
                 hw_config.left_hand_pin, hw_config.right_hand_pin);

        LoadTrimsFromNVS();

        action_queue_ = xQueueCreate(10, sizeof(OttoActionParams));

        QueueAction(ACTION_HOME, 1, 1000, 1, 0); // Direction=1 means reset the hand

        RegisterMcpTools();
    }

    void RegisterMcpTools()
    {
        auto &mcp_server = McpServer::GetInstance();

        ESP_LOGI(TAG, "Start registering mcp tool...");

        // Unified action tool (all actions except servo sequences)

        mcp_server.AddTool("self.otto.action",
                           "Perform robot actions. action: action name; provide corresponding parameters according to the action type: direction: direction, 1=forward/turn left, -1=backward/turn right; 0=left and right at the same time"
                           "steps: number of action steps, 1-100; speed: action speed, 100-3000, the smaller the value, the faster; amount: range of action, 0-170; arm_swing: arm swing range, 0-170;"
                           "Basic movements: walk (walking, steps/speed/direction/arm_swing required), turn (turning, steps/speed/direction/arm_swing required), jump (jumping, steps/speed required),"
                           "swing (swing, requires steps/speed/amount), moonwalk (moonwalk, requires steps/speed/direction/amount), bend (bend, requires steps/speed/direction),"
                           "shake_leg (shake leg, steps/speed/direction required), updown (up and down movement, steps/speed/amount required), whirlwind_leg (whirlwind leg, steps/speed/amount required);"
                           "Fixed actions: sit (sit down), showcase (show action), home (reset);"
                           "Hand movements (requires hand servo): hands_up (raises hand, requires speed/direction), hands_down (lets go, requires speed/direction), hand_wave (waves, requires direction),"
                           "windmill (big windmill, steps/speed/amount required), takeoff (takeoff, steps/speed/amount required), fitness (fitness, steps/speed/amount required),"
                           "greeting (greeting, direction/steps required), shy (shy, direction/steps required), radio_calisthenics (radio gymnastics), magic_circle (magic circle of love)",
                           PropertyList({Property("action", kPropertyTypeString, "sit"),
                                         Property("steps", kPropertyTypeInteger, 3, 1, 100),
                                         Property("speed", kPropertyTypeInteger, 700, 100, 3000),
                                         Property("direction", kPropertyTypeInteger, 1, -1, 1),
                                         Property("amount", kPropertyTypeInteger, 30, 0, 170),
                                         Property("arm_swing", kPropertyTypeInteger, 50, 0, 170)}),
                           [this](const PropertyList &properties) -> ReturnValue
                           {
                               std::string action = properties["action"].value<std::string>();
                               // All parameters have default values ​​and can be accessed directly

                               int steps = properties["steps"].value<int>();
                               int speed = properties["speed"].value<int>();
                               int direction = properties["direction"].value<int>();
                               int amount = properties["amount"].value<int>();
                               int arm_swing = properties["arm_swing"].value<int>();

                               // Basic movement actions

                               if (action == "walk")
                               {
                                   QueueAction(ACTION_WALK, steps, speed, direction, arm_swing);
                                   return true;
                               }
                               else if (action == "turn")
                               {
                                   QueueAction(ACTION_TURN, steps, speed, direction, arm_swing);
                                   return true;
                               }
                               else if (action == "jump")
                               {
                                   QueueAction(ACTION_JUMP, steps, speed, 0, 0);
                                   return true;
                               }
                               else if (action == "swing")
                               {
                                   QueueAction(ACTION_SWING, steps, speed, 0, amount);
                                   return true;
                               }
                               else if (action == "moonwalk")
                               {
                                   QueueAction(ACTION_MOONWALK, steps, speed, direction, amount);
                                   return true;
                               }
                               else if (action == "bend")
                               {
                                   QueueAction(ACTION_BEND, steps, speed, direction, 0);
                                   return true;
                               }
                               else if (action == "shake_leg")
                               {
                                   QueueAction(ACTION_SHAKE_LEG, steps, speed, direction, 0);
                                   return true;
                               }
                               else if (action == "updown")
                               {
                                   QueueAction(ACTION_UPDOWN, steps, speed, 0, amount);
                                   return true;
                               }
                               else if (action == "whirlwind_leg")
                               {
                                   QueueAction(ACTION_WHIRLWIND_LEG, steps, speed, 0, amount);
                                   return true;
                               }
                               // Fixed action

                               else if (action == "sit")
                               {
                                   QueueAction(ACTION_SIT, 1, 0, 0, 0);
                                   return true;
                               }
                               else if (action == "showcase")
                               {
                                   QueueAction(ACTION_SHOWCASE, 1, 0, 0, 0);
                                   return true;
                               }
                               else if (action == "home")
                               {
                                   QueueAction(ACTION_HOME, 1, 1000, 1, 0);
                                   return true;
                               }
                               // hand movements

                               else if (action == "hands_up")
                               {
                                   if (!has_hands_)
                                   {
                                       return "Error: This action requires hand servo support";
                                   }
                                   QueueAction(ACTION_HANDS_UP, 1, speed, direction, 0);
                                   return true;
                               }
                               else if (action == "hands_down")
                               {
                                   if (!has_hands_)
                                   {
                                       return "Error: This action requires hand servo support";
                                   }
                                   QueueAction(ACTION_HANDS_DOWN, 1, speed, direction, 0);
                                   return true;
                               }
                               else if (action == "hand_wave")
                               {
                                   if (!has_hands_)
                                   {
                                       return "Error: This action requires hand servo support";
                                   }
                                   QueueAction(ACTION_HAND_WAVE, 1, 0, 0, direction);
                                   return true;
                               }
                               else if (action == "windmill")
                               {
                                   if (!has_hands_)
                                   {
                                       return "Error: This action requires hand servo support";
                                   }
                                   QueueAction(ACTION_WINDMILL, steps, speed, 0, amount);
                                   return true;
                               }
                               else if (action == "takeoff")
                               {
                                   if (!has_hands_)
                                   {
                                       return "Error: This action requires hand servo support";
                                   }
                                   QueueAction(ACTION_TAKEOFF, steps, speed, 0, amount);
                                   return true;
                               }
                               else if (action == "fitness")
                               {
                                   if (!has_hands_)
                                   {
                                       return "Error: This action requires hand servo support";
                                   }
                                   QueueAction(ACTION_FITNESS, steps, speed, 0, amount);
                                   return true;
                               }
                               else if (action == "greeting")
                               {
                                   if (!has_hands_)
                                   {
                                       return "Error: This action requires hand servo support";
                                   }
                                   QueueAction(ACTION_GREETING, steps, 0, direction, 0);
                                   return true;
                               }
                               else if (action == "shy")
                               {
                                   if (!has_hands_)
                                   {
                                       return "Error: This action requires hand servo support";
                                   }
                                   QueueAction(ACTION_SHY, steps, 0, direction, 0);
                                   return true;
                               }
                               else if (action == "radio_calisthenics")
                               {
                                   if (!has_hands_)
                                   {
                                       return "Error: This action requires hand servo support";
                                   }
                                   QueueAction(ACTION_RADIO_CALISTHENICS, 1, 0, 0, 0);
                                   return true;
                               }
                               else if (action == "magic_circle")
                               {
                                   if (!has_hands_)
                                   {
                                       return "Error: This action requires hand servo support";
                                   }
                                   QueueAction(ACTION_MAGIC_CIRCLE, 1, 0, 0, 0);
                                   return true;
                               }
                               else
                               {
                                   return "Error: Invalid action name. Available actions: walk, turn, jump, swing, moonwalk, bend, shake_leg, updown, whirlwind_leg, sit, showcase, home, hands_up, hands_down, hand_wave, windmill, takeoff, fitness, greeting, shy, radio_calisthenics, magic_circle";
                               }
                           });

        // Servo sequence tool (supports segmented sending, sending one sequence at a time and automatically queuing for execution)

        mcp_server.AddTool(
            "self.otto.servo_sequences",
            "AI custom action programming (improvisational actions). Supports sending sequences in segments: For more than 5 sequences, it is recommended that the AI ​​can call this tool multiple times in a row. Each time a short sequence is sent, the system will automatically queue it up and execute it in sequence. Supports two modes: normal movement and oscillator. "
            "Robot structure: The hands can swing up and down, the legs can be retracted and abducted, and the feet can be flipped up and down. "
            "Servo gear description:"
            "ll (left leg): adduction and abduction, 0 degrees = full abduction, 90 degrees = neutral, 180 degrees = full adduction;"
            "rl (right leg): adduction and abduction, 0 degrees = full adduction, 90 degrees = neutral, 180 degrees = full abduction;"
            "lf (left foot): flip up and down, 0 degrees = completely upward, 90 degrees = horizontal, 180 degrees = completely downward;"
            "rf (right foot): flip up and down, 0 degrees = completely downward, 90 degrees = horizontal, 180 degrees = completely upward;"
            "lh (left hand): swing up and down, 0 degrees = completely downward, 90 degrees = horizontal, 180 degrees = completely upward;"
            "rh (right hand): swing up and down, 0 degrees = completely upward, 90 degrees = horizontal, 180 degrees = completely downward;"
"sequence: a single sequence object, containing the 'a' action array, top-level optional parameters:"
            "'d' (delay in milliseconds after sequence execution completes, used for pauses between sequences)."
            "Each action object contains:"
            "Normal mode: 's' servo position object (key name: ll/rl/lf/rf/lh/rh, value: 0-180 degrees), 'v' movement speed 100-3000 milliseconds (default 1000), 'd' delay milliseconds after action (default 0);"
            "Oscillation mode: 'osc' oscillator object, including 'a' amplitude object (amplitude of each servo 10-90 degrees, default 20 degrees), 'o' center angle object (absolute angle of oscillation center of each servo 0-180 degrees, default 90 degrees) ), 'ph' phase difference object (phase difference of each servo, degrees, 0-360 degrees, default 0 degrees), 'p' period 100-3000 milliseconds (default 500), 'c' period number 0.1-20.0 (default 5.0); "
            "How to use: AI can call this tool multiple times in a row. Each time a sequence is sent, the system will automatically queue it and execute it in order."
"Important note: When the left and right legs vibrate, one foot must be at 90 degrees, otherwise the robot will be damaged. If multiple sequences are sent (sequence number > 1), and when a reset is required after completing all sequences, the AI should finally call the self.otto.home tool to reset alone. Do not set reset parameters in the sequence."
            "Normal mode example: send 3 sequences and finally call reset:"
            "The first call {\"sequence\":\"{\\\"a\\\":[{\\\"s\\\":{\\\"ll\\\":100},\\\"v\\\":1000}],\\\"d\\\":500}\"},"
            "The second call {\"sequence\":\"{\\\"a\\\":[{\\\"s\\\":{\\\"ll\\\":90},\\\"v\\\":800}],\\\"d\\\":500}\"},"
            "The third call {\"sequence\":\"{\\\"a\\\":[{\\\"s\\\":{\\\"ll\\\":80},\\\"v\\\":800}]}\"},"
"Finally call the self.otto.home tool to reset."
            "Oscillator mode example:"
            "Example 1 -Arms swing synchronously: {\"sequence\":\"{\\\"a\\\":[{\\\"osc\\\":{\\\"a\\\":{\\\"lh\\\":30,\\\"rh\\\":3 0},\\\"o\\\":{\\\"lh\\\":90,\\\"rh\\\":-90},\\\"p\\\":500,\\\"c\\\":5.0}}],\\\"d\\\":0}\"};"
            "Example 2 -Legs oscillate alternately (wave effect): {\"sequence\":\"{\\\"a\\\":[{\\\"osc\\\":{\\\"a\\\":{\\\"ll\\\":20,\\\"rl\\\":20},\\\"o\\\ ":{\\"ll\\\":90,\\\"rl\\\":-90},\\\"ph\\\":{\\\"rl\\\":180},\\\"p\\\":600,\\\"c\\\":3.0}}],\\\"d\\\":0}\"};"
"Example 3 -Single leg oscillation with fixed feet (safety): {\"sequence\":\"{\\\"a\\\":[{\\\"osc\\\":{\\\"a\\\":{\\\"ll\\\":45}, \\\"o\\\":{\\\"ll\\\":90,\\\"lf\\\":90},\\\"p\\\":400,\\\"c\\\":4.0}}],\\\"d\\\":0}\"};"
            "Example 4 -Complex multi-servo oscillation (hands and legs): {\"sequence\":\"{\\\"a\\\":[{\\\"osc\\\":{\\\"a \\\":{\\\"lh\\\":25,\\\"rh\\\":25,\\\"ll\\\":15},\\\"o\\\":{\\\"l h\\\":90,\\\"rh\\\":90,\\\"ll\\\":90,\\\"lf\\\":90},\\\"ph\\\":{\ \\"rh\\\":180},\\\"p\\\":800,\\\"c\\\":6.0}}],\\\"d\\\":500}\"};"
"Example 5 -Quick swing: {\"sequence\":\"{\\\"a\\\":[{\\\"osc\\\":{\\\"a\\\":{\\\"ll\\\":30,\\\"rl\\\":30},\\\"o\\\":{\ \\"ll\\\":90,\\\"rl\\\":90},\\\"ph\\\":{\\\"rl\\\":180},\\\"p\\\":300,\\\"c\\\":10.0}}],\\\"d\\\":0}\"}.",
            PropertyList({Property("sequence", kPropertyTypeString,
                                   "{\"a\":[{\"s\":{\"ll\":90,\"rl\":90},\"v\":1000}]}")}),
            [this](const PropertyList& properties) -> ReturnValue {
                std::string sequence = properties["sequence"].value<std::string>();
                // Check if it is a JSON object (maybe in string format or a parsed object)
                // If sequence is a JSON string, use it directly; if it is an object string, you also need to use it.

                QueueServoSequence(sequence.c_str());
                return true;
            });

            mcp_server.AddTool("self.otto.stop", "Immediately stop all actions and reset", PropertyList(),
                               [this](const PropertyList &properties) -> ReturnValue
                               {
                                   if (action_task_handle_ != nullptr)
                                   {
                                       vTaskDelete(action_task_handle_);
                                       action_task_handle_ = nullptr;
                                   }
                                   is_action_in_progress_ = false;
                                   PowerManager::ResumeBatteryUpdate(); // 停止动作时恢复电量更新
                                   xQueueReset(action_queue_);

                                   QueueAction(ACTION_HOME, 1, 1000, 1, 0);
                                   return true;
                               });

            mcp_server.AddTool(
                "self.otto.set_trim",
                "Calibrate individual servo positions. Set the fine-tuning parameters of the specified servo to adjust the robot's initial standing posture. The settings will be permanently saved."
                "servo_type: Servo type(left_leg/right_leg/left_foot/right_foot/left_hand/right_hand); "
                "trim_value: Fine adjustment value (50 to 50 degrees)",
                PropertyList({Property("servo_type", kPropertyTypeString, "left_leg"),
                              Property("trim_value", kPropertyTypeInteger, 0, -50, 50)}),
                [this](const PropertyList &properties) -> ReturnValue
                {
                    std::string servo_type = properties["servo_type"].value<std::string>();
                    int trim_value = properties["trim_value"].value<int>();

                    ESP_LOGI(TAG, "Set servo trim: %s = %d度", servo_type.c_str(), trim_value);

                    // Get all current trimming values

                    Settings settings("otto_trims", true);
                    int left_leg = settings.GetInt("left_leg", 0);
                    int right_leg = settings.GetInt("right_leg", 0);
                    int left_foot = settings.GetInt("left_foot", 0);
                    int right_foot = settings.GetInt("right_foot", 0);
                    int left_hand = settings.GetInt("left_hand", 0);
                    int right_hand = settings.GetInt("right_hand", 0);

                    // Update the trim value of the specified servo

                    if (servo_type == "left_leg")
                    {
                        left_leg = trim_value;
                        settings.SetInt("left_leg", left_leg);
                    }
                    else if (servo_type == "right_leg")
                    {
                        right_leg = trim_value;
                        settings.SetInt("right_leg", right_leg);
                    }
                    else if (servo_type == "left_foot")
                    {
                        left_foot = trim_value;
                        settings.SetInt("left_foot", left_foot);
                    }
                    else if (servo_type == "right_foot")
                    {
                        right_foot = trim_value;
                        settings.SetInt("right_foot", right_foot);
                    }
                    else if (servo_type == "left_hand")
                    {
                        if (!has_hands_)
                        {
                            return "Error: The robot is not configured with hand servo";
                        }
                        left_hand = trim_value;
                        settings.SetInt("left_hand", left_hand);
                    }
                    else if (servo_type == "right_hand")
                    {
                        if (!has_hands_)
                        {
                            return "Error: The robot is not configured with hand servo";
                        }
                        right_hand = trim_value;
                        settings.SetInt("right_hand", right_hand);
                    }
                    else
                    {
                        return "Error: Invalid servo type, please use: left_leg, right_leg, left_foot, "
                               "right_foot, left_hand, right_hand";
                    }

                    otto_.SetTrims(left_leg, right_leg, left_foot, right_foot, left_hand, right_hand);

                    QueueAction(ACTION_JUMP, 1, 500, 0, 0);

                    return "steering gear " + servo_type + " Fine-tuning is set to " + std::to_string(trim_value) +
                           " degree, permanently saved";
                });

            mcp_server.AddTool("self.otto.get_trims", "Get the current servo trim settings", PropertyList(),
                               [this](const PropertyList &properties) -> ReturnValue
                               {
                                   Settings settings("otto_trims", false);

                                   int left_leg = settings.GetInt("left_leg", 0);
                                   int right_leg = settings.GetInt("right_leg", 0);
                                   int left_foot = settings.GetInt("left_foot", 0);
                                   int right_foot = settings.GetInt("right_foot", 0);
                                   int left_hand = settings.GetInt("left_hand", 0);
                                   int right_hand = settings.GetInt("right_hand", 0);

                                   std::string result =
                                       "{\"left_leg\":" + std::to_string(left_leg) +
                                       ",\"right_leg\":" + std::to_string(right_leg) +
                                       ",\"left_foot\":" + std::to_string(left_foot) +
                                       ",\"right_foot\":" + std::to_string(right_foot) +
                                       ",\"left_hand\":" + std::to_string(left_hand) +
                                       ",\"right_hand\":" + std::to_string(right_hand) + "}";

                                   ESP_LOGI(TAG, "Get fine-tuning settings: %s", result.c_str());
                                   return result;
                               });

            mcp_server.AddTool("self.otto.get_status", "Get the robot status, return moving or idle",
                               PropertyList(), [this](const PropertyList &properties) -> ReturnValue
                               { return is_action_in_progress_ ? "moving" : "idle"; });

            mcp_server.AddTool("self.battery.get_level", "Get the robot battery power and charging status", PropertyList(),
                               [](const PropertyList &properties) -> ReturnValue
                               {
                                   auto &board = Board::GetInstance();
                                   int level = 0;
                                   bool charging = false;
                                   bool discharging = false;
                                   board.GetBatteryLevel(level, charging, discharging);

                                   std::string status =
                                       "{\"level\":" + std::to_string(level) +
                                       ",\"charging\":" + (charging ? "true" : "false") + "}";
                                   return status;
                               });

            mcp_server.AddTool("self.otto.get_ip", "获取机器人WiFi IP地址", PropertyList(),
                               [](const PropertyList &properties) -> ReturnValue
                               {
                                   auto &wifi = WifiManager::GetInstance();
                                   std::string ip = wifi.GetIpAddress();
                                   if (ip.empty())
                                   {
                                       return "{\"ip\":\"\",\"connected\":false}";
                                   }
                                   std::string status = "{\"ip\":\"" + ip + "\",\"connected\":true}";
                                   return status;
                               });

            ESP_LOGI(TAG, "Mcp tool registration completed");
    }

    ~OttoController() {
            if (action_task_handle_ != nullptr)
            {
                vTaskDelete(action_task_handle_);
                action_task_handle_ = nullptr;
            }
            vQueueDelete(action_queue_);
    }
    };

    static OttoController *g_otto_controller = nullptr;

    void InitializeOttoController(const HardwareConfig &hw_config)
    {
        if (g_otto_controller == nullptr)
        {
            g_otto_controller = new OttoController(hw_config);
            ESP_LOGI(TAG, "Otto控制器已初始化并注册MCP工具");
        }
    }
