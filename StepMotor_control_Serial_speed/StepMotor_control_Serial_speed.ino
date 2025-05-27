#include <AccelStepper.h>

// —————————— 步进电机相关定义 ——————————
// 使用 DRIVER 模式（STEP + DIR 控制）
#define STEP_PIN   5
#define DIR_PIN    6
#define ENA_PIN    7

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// 步进电机参数
// 1.8°/步 = 200 步/圈；8 微步 ⇒ 200 × 8 = 1600 步/圈
const int MICROSTEPS    = 16;
const int STEPS_PER_REV = 200 * MICROSTEPS;  // 1600

// 允许的最大转速（自行根据电机规格调整）。
// 这里假设最高 240 RPM，对应：240/60*1600 = 6400 步/秒
const long MAX_STEPS_PER_SEC = (600L)/60*STEPS_PER_REV;

void setup() {
  // 初始化串口，用于接收速度指令和状态打印
  Serial.begin(115200);

  // 配置使能引脚
  pinMode(ENA_PIN, OUTPUT);
  // 默认关闭驱动（HIGH = 关闭）
  digitalWrite(ENA_PIN, HIGH);

  // 设置步进电机最大速度（不涉及加速/减速，直接匀速控制）
  stepper.setMaxSpeed(MAX_STEPS_PER_SEC);
}

void loop() {
  static long currentStepsPerSec = 0;
  static long currentRPM = 0;
  static unsigned long lastPrintTime = 0;

  // —— 1. 检查串口是否有新指令 —— 
  if (Serial.available()) {
    // 读取直到换行符或超时
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();           // 去掉首尾空白
    cmd.toLowerCase();    // 转小写，方便后续判断

    // 期望格式：s<rpm>，例如 “s60” 或 “S60” 均可
    if (cmd.length() > 1 && cmd.charAt(0) == 's') {
      // 提取数字部分
      long rpm = cmd.substring(1).toInt();
      if (rpm < 0) {
        rpm = 0;
      }

      // 计算对应的步/秒： stepsPerSec = rpm * STEPS_PER_REV / 60
      long stepsPerSec = (rpm * STEPS_PER_REV) / 60;
      if (stepsPerSec > MAX_STEPS_PER_SEC) {
        stepsPerSec = MAX_STEPS_PER_SEC;  // 限制在最大值内
        // 将 rpm 对应调整到最大可实现：MAX_STEPS_PER_SEC * 60 / STEPS_PER_REV
        rpm = (MAX_STEPS_PER_SEC * 60) / STEPS_PER_REV;
      }

      // 如果 rpm > 0，则开启电机；否则关闭电机
      if (stepsPerSec > 0) {
        digitalWrite(ENA_PIN, LOW);          // 使能驱动器（LOW = 打开）
        stepper.setSpeed(stepsPerSec);       // 设置目标速度（正向）
        currentStepsPerSec = stepsPerSec;
        currentRPM = rpm;
      } else {
        // 当 rpm == 0 时，停止电机并关闭驱动
        stepper.setSpeed(0);
        currentStepsPerSec = 0;
        currentRPM = 0;
        digitalWrite(ENA_PIN, HIGH);         // 关闭驱动器（HIGH = 关闭）
      }
    }
  }

  // —— 2. 如果电机应该运行，则保持匀速转动 —— 
  if (currentStepsPerSec != 0) {
    stepper.runSpeed();
  }

  // —— 3. 每隔 1 秒打印当前状态 —— 
  unsigned long now = millis();
  if (now - lastPrintTime >= 1000) {
    lastPrintTime = now;

    // 判断电机是否正在使能（低电平表示使能打开）
    bool enabled = (digitalRead(ENA_PIN) == LOW);

    // 打印格式示例:
    // 状态: 运行, 速度: 60 RPM, 步/秒: 1600
    // 或
    // 状态: 停止, 速度: 0 RPM
    Serial.print("状态: ");
    if (enabled && currentRPM > 0) {
      Serial.print("运行, ");
      Serial.print("速度: ");
      Serial.print(currentRPM);
      Serial.println(" RPM");

    } else {
      Serial.print("停止, ");
      Serial.println("速度: 0 RPM");
    }
  }
}
