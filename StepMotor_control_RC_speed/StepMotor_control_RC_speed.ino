#include <AccelStepper.h>
#include <IBusBM.h>  // iBus 库

// —————————— 步进电机控制引脚定义 ——————————
#define M1_STEP_PIN   3
#define M1_DIR_PIN    4
#define M1_ENA_PIN    7

#define M2_STEP_PIN   5
#define M2_DIR_PIN    6
#define M2_ENA_PIN    7

#define M3_STEP_PIN   9
#define M3_DIR_PIN    8
#define M3_ENA_PIN    7

#define M4_STEP_PIN   10
#define M4_DIR_PIN    11
#define M4_ENA_PIN    7

AccelStepper stepper1(AccelStepper::DRIVER, M1_STEP_PIN, M1_DIR_PIN);
AccelStepper stepper2(AccelStepper::DRIVER, M2_STEP_PIN, M2_DIR_PIN);
AccelStepper stepper3(AccelStepper::DRIVER, M3_STEP_PIN, M3_DIR_PIN);
AccelStepper stepper4(AccelStepper::DRIVER, M4_STEP_PIN, M4_DIR_PIN);

// —————————— 参数设置 ——————————
const int MICROSTEPS     = 8;
const int STEPS_PER_REV  = 200 * MICROSTEPS;   // 1600 steps/rev
const long MAX_SPEED    = (120L / 60) * STEPS_PER_REV;  // 6400 steps/sec

// 死区设置
const int DEADZONE_LOWER = 1490;
const int DEADZONE_UPPER = 1510;

// iBus 设置
IBusBM ibus;
const uint8_t IBUS_CH1 = 0;
const uint8_t IBUS_CH2 = 1;
const uint8_t IBUS_CH3 = 2;
const uint8_t IBUS_CH4 = 3;

void setup() {
  Serial.begin(115200);
  ibus.begin(Serial);

  // 设置使能引脚为输出并拉低（使能电机）
  pinMode(M1_ENA_PIN, OUTPUT);
  digitalWrite(M1_ENA_PIN, LOW);

  // 电机最大速度设置
  stepper1.setMaxSpeed(MAX_SPEED);
  stepper2.setMaxSpeed(MAX_SPEED);
  stepper3.setMaxSpeed(MAX_SPEED);
  stepper4.setMaxSpeed(MAX_SPEED);

  delay(300); // 等待接收机初始化
}

void loop() {
  uint16_t ch1 = ibus.readChannel(IBUS_CH1);
  uint16_t ch2 = ibus.readChannel(IBUS_CH2);
  uint16_t ch3 = ibus.readChannel(IBUS_CH3);
  uint16_t ch4 = ibus.readChannel(IBUS_CH4);

  // 控制每个电机速度
  stepper1.setSpeed(mapWithDeadzone(ch1));
  stepper2.setSpeed(mapWithDeadzone(ch2));
  stepper3.setSpeed(mapWithDeadzone(ch3));
  stepper4.setSpeed(mapWithDeadzone(ch4));

  // 驱动运行（必须在 loop 中持续调用）
  stepper1.runSpeed();
  stepper2.runSpeed();
  stepper3.runSpeed();
  stepper4.runSpeed();
  
}

// 映射函数：将 iBus 通道值转换为速度（带死区）
long mapWithDeadzone(uint16_t val) {
  if (val < 1000 || val > 2000) return 0;

  if (val >= DEADZONE_LOWER && val <= DEADZONE_UPPER) return 0;

  // 从 [1000, 2000] 映射到 [-MAX_SPEED, +MAX_SPEED]
  return map(val, 1000, 2000, -MAX_SPEED, MAX_SPEED);
}
