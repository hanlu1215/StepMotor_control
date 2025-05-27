#include <AccelStepper.h>
#include <IBusBM.h>     // 引入 IBusBM 库

// —————————— 步进电机相关定义 ——————————
// 这里使用 DRIVER 模式（STEP + DIR 控制）
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

// 为四个电机创建 AccelStepper 实例
AccelStepper stepper1(AccelStepper::DRIVER, M1_STEP_PIN, M1_DIR_PIN);
AccelStepper stepper2(AccelStepper::DRIVER, M2_STEP_PIN, M2_DIR_PIN);
AccelStepper stepper3(AccelStepper::DRIVER, M3_STEP_PIN, M3_DIR_PIN);
AccelStepper stepper4(AccelStepper::DRIVER, M4_STEP_PIN, M4_DIR_PIN);

// 步进电机参数
// 假设 1.8°/步 = 200 步/圈；8 微步 ⇒ 200 × 8 = 1600 步/圈
const int MICROSTEPS       = 16;
const int STEPS_PER_REV    = 200 * MICROSTEPS;  // 1600

// —————————— IBusBM 相关定义 ——————————
IBusBM ibus;                // IBusBM 对象
// iBus 通道索引：Channel1→index0, Channel2→index1, Channel3→index2, Channel4→index3
const uint8_t IBUS_CH1 = 0;
const uint8_t IBUS_CH2 = 1;
const uint8_t IBUS_CH3 = 2;
const uint8_t IBUS_CH4 = 3;

// ———————— setup() ————————
void setup() {
  // 1. 初始化 Serial（115200 波特）用于接收 iBus 数据
  Serial.begin(115200);
  // 让 IBusBM 监听硬件串口 Serial
  ibus.begin(Serial);

  // 2. 配置四个电机的使能引脚
  pinMode(M1_ENA_PIN, OUTPUT);
  pinMode(M2_ENA_PIN, OUTPUT);
  pinMode(M3_ENA_PIN, OUTPUT);
  pinMode(M4_ENA_PIN, OUTPUT);
  // LOW = 使能打开
  digitalWrite(M1_ENA_PIN, LOW);
  digitalWrite(M2_ENA_PIN, LOW);
  digitalWrite(M3_ENA_PIN, LOW);
  digitalWrite(M4_ENA_PIN, LOW);

  // 3. 设置四个电机的最大速度和加速度（可按需再调整）
  //    这里统一设数字，实际可针对每个电机写不同数值
  stepper1.setMaxSpeed(20000);   // 10000 步/秒 ≈ 375 RPM
  stepper1.setAcceleration(15000);// 5000 步/秒²

  stepper2.setMaxSpeed(20000);
  stepper2.setAcceleration(15000);

  stepper3.setMaxSpeed(20000);
  stepper3.setAcceleration(15000);

  stepper4.setMaxSpeed(20000);
  stepper4.setAcceleration(15000);

  // 4. 等待 iBus 接收机稳定
  delay(300);
}

// ———————— loop() ————————
void loop() {
  // —— 1. 读取 iBus 前四个通道的原始数值 —— 
  //    readChannel() 若此次没有新帧或 CRC 不通过，会返回 0
  uint16_t raw1 = ibus.readChannel(IBUS_CH1);
  uint16_t raw2 = ibus.readChannel(IBUS_CH2);
  uint16_t raw3 = ibus.readChannel(IBUS_CH3);
  uint16_t raw4 = ibus.readChannel(IBUS_CH4);

  // —— 2. 对每个通道分别做映射并更新各自的目标步数 —— 
  //    只有当读到 [1000,2000] 范围才映射，否则保持上次目标
  if (raw1 >= 1000 && raw1 <= 2000) {
    // 通道1 映射到 Motor1：1000→0 步，2000→1600 步（1圈）
    long target1 = map(raw1, 1000, 2000, 0, STEPS_PER_REV);
    stepper1.moveTo(target1);
  }
  if (raw2 >= 1000 && raw2 <= 2000) {
    // 通道2 映射到 Motor2
    long target2 = map(raw2, 1000, 2000, 0, STEPS_PER_REV);
    stepper2.moveTo(target2);
  }
  if (raw3 >= 1000 && raw3 <= 2000) {
    // 通道3 映射到 Motor3
    long target3 = map(raw3, 1000, 2000, 0, STEPS_PER_REV);
    stepper3.moveTo(target3);
  }
  if (raw4 >= 1000 && raw4 <= 2000) {
    // 通道4 映射到 Motor4
    long target4 = map(raw4, 1000, 2000, 0, STEPS_PER_REV);
    stepper4.moveTo(target4);
  }

  // —— 3. 让四个电机平滑运动 —— 
  //    必须连续调用 run()，库会处理加速和减速
  stepper1.run();
  stepper2.run();
  stepper3.run();
  stepper4.run();
}
