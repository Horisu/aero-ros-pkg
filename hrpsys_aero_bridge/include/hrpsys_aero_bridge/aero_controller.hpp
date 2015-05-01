#ifndef HRPSYS_AERO_BRIDGE_AERO_CONTROLLER_HPP_
#define HRPSYS_AERO_BRIDGE_AERO_CONTROLLER_HPP_

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <stdint.h>
#include <unistd.h>

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost::asio;

namespace {
// header offset = 6bytes
const static size_t RAW_HEADER_OFFSET = 6;
// data length = 77bytes
const static size_t RAW_DATA_LENGTH = 77;
// id: upper = 1, lower = 2
const static uint8_t ID_UPPER = 1;
const static uint8_t ID_LOWER = 2;

// whole: 46DOF
//  upper: 22DOF
//   neck: 3DOF
//   arm: 8DOF (including hand) * 2
//    shoulder: 2DOF * 2
//    elbow: 2DOF * 2
//    wrist: 3DOF * 2
//    hand: 1DOF * 2
//   waist: 3DOF
//  lower (4LEGS) : 24DOF
//   leg: 6DOF * 4
//    crotch: 3DOF * 4
//    knee: 2DOF * 4
//    wheel: 1DOF * 4

const static size_t AERO_DOF = 46;
const static size_t AERO_DOF_UPPER = 22;
const static size_t AERO_DOF_LOWER = 24;

// joint index in stroke vector
// UPPER
const static size_t STROKE_NECK_Y = 0;
const static size_t STROKE_NECK_RIGHT = 1;
const static size_t STROKE_NECK_LEFT = 2;

const static size_t STROKE_RIGHT_SHOULDER_P = 3;
const static size_t STROKE_RIGHT_SHOULDER_R = 4;
const static size_t STROKE_RIGHT_ELBOW_Y = 5;
const static size_t STROKE_RIGHT_ELBOW_P = 6;
const static size_t STROKE_RIGHT_WRIST_R = 7;
const static size_t STROKE_RIGHT_WRIST_TOP = 8;
const static size_t STROKE_RIGHT_WRIST_BOTTOM = 9;
const static size_t STROKE_RIGHT_HAND = 10;

const static size_t STROKE_LEFT_SHOULDER_P = 11;
const static size_t STROKE_LEFT_SHOULDER_R = 12;
const static size_t STROKE_LEFT_ELBOW_Y = 13;
const static size_t STROKE_LEFT_ELBOW_P = 14;
const static size_t STROKE_LEFT_WRIST_R = 15;
const static size_t STROKE_LEFT_WRIST_TOP = 16;
const static size_t STROKE_LEFT_WRIST_BOTTOM = 17;
const static size_t STROKE_LEFT_HAND = 18;

const static size_t STROKE_WAIST_RIGHT = 19;
const static size_t STROKE_WAIST_LEFT = 20;
const static size_t STROKE_WAIST_P = 21;
// LOWER (4 LEGS):
const static size_t STROKE_FRONT_RIGHT_CROTCH_Y = 22;
const static size_t STROKE_FRONT_RIGHT_CROTCH_P0 = 23;
const static size_t STROKE_FRONT_RIGHT_CROTCH_P1 = 24;
const static size_t STROKE_FRONT_RIGHT_KNEE_P0 = 25;
const static size_t STROKE_FRONT_RIGHT_KNEE_P1 = 26;
const static size_t STROKE_FRONT_RIGHT_WHEEL = 27;

const static size_t STROKE_REAR_RIGHT_CROTCH_Y = 28;
const static size_t STROKE_REAR_RIGHT_CROTCH_P0 = 29;
const static size_t STROKE_REAR_RIGHT_CROTCH_P1 = 30;
const static size_t STROKE_REAR_RIGHT_KNEE_P0 = 31;
const static size_t STROKE_REAR_RIGHT_KNEE_P1 = 32;
const static size_t STROKE_REAR_RIGHT_WHEEL = 33;

const static size_t STROKE_FRONT_LEFT_CROTCH_Y = 34;
const static size_t STROKE_FRONT_LEFT_CROTCH_P0 = 35;
const static size_t STROKE_FRONT_LEFT_CROTCH_P1 = 36;
const static size_t STROKE_FRONT_LEFT_KNEE_P0 = 37;
const static size_t STROKE_FRONT_LEFT_KNEE_P1 = 38;
const static size_t STROKE_FRONT_LEFT_WHEEL = 39;

const static size_t STROKE_REAR_LEFT_CROTCH_Y = 40;
const static size_t STROKE_REAR_LEFT_CROTCH_P0 = 41;
const static size_t STROKE_REAR_LEFT_CROTCH_P1 = 42;
const static size_t STROKE_REAR_LEFT_KNEE_P0 = 43;
const static size_t STROKE_REAR_LEFT_KNEE_P1 = 44;
const static size_t STROKE_REAR_LEFT_WHEEL = 45;


// joint index in raw vector (as int16_t)
// UPPER: ID = 1
const static size_t RAW_NECK_Y = 0;
const static size_t RAW_NECK_RIGHT = 1;
const static size_t RAW_NECK_LEFT = 2;
const static size_t RAW_RIGHT_SHOULDER_P = 3;
const static size_t RAW_RIGHT_SHOULDER_R = 4;
const static size_t RAW_RIGHT_ELBOW_Y = 5;
const static size_t RAW_RIGHT_ELBOW_P = 6;
const static size_t RAW_RIGHT_WRIST_R = 7;
const static size_t RAW_RIGHT_WRIST_TOP = 8;
const static size_t RAW_RIGHT_WRIST_BOTTOM = 9;
const static size_t RAW_WAIST_RIGHT = 10;
const static size_t RAW_RIGHT_HAND = 11;
// 12 - 15: Force Sensor (uint8_t * 6, 2bytes N/A)
const static size_t RAW_WAIST_P = 16;
// 17 - 18: N/A
const static size_t RAW_LEFT_SHOULDER_P = 19;
const static size_t RAW_LEFT_SHOULDER_R = 20;
const static size_t RAW_LEFT_ELBOW_Y = 21;
const static size_t RAW_LEFT_ELBOW_P = 22;
const static size_t RAW_LEFT_WRIST_R = 23;
const static size_t RAW_LEFT_WRIST_TOP = 24;
const static size_t RAW_LEFT_WRIST_BOTTOM = 25;
const static size_t RAW_WAIST_LEFT = 26;
const static size_t RAW_LEFT_HAND = 27;
// 28 - 31: Force Sensor (uint8_t * 6, 2bytes N/A)
// 32 - 34: N/A

// LOWER (4 LEGS):
const static size_t RAW_FRONT_RIGHT_CROTCH_Y = 0;
const static size_t RAW_FRONT_RIGHT_CROTCH_P0 = 1;
const static size_t RAW_FRONT_RIGHT_CROTCH_P1 = 2;
const static size_t RAW_FRONT_RIGHT_KNEE_P0 = 3;
const static size_t RAW_FRONT_RIGHT_KNEE_P1 = 4;
const static size_t RAW_FRONT_RIGHT_WHEEL = 5;
const static size_t RAW_REAR_RIGHT_CROTCH_Y = 6;
const static size_t RAW_REAR_RIGHT_CROTCH_P0 = 7;
const static size_t RAW_REAR_RIGHT_CROTCH_P1 = 8;
const static size_t RAW_REAR_RIGHT_KNEE_P0 = 9;
const static size_t RAW_REAR_RIGHT_KNEE_P1 = 10;
const static size_t RAW_REAR_RIGHT_WHEEL = 11;
// 12 - 15: N/A
const static size_t RAW_FRONT_LEFT_CROTCH_Y = 16;
const static size_t RAW_FRONT_LEFT_CROTCH_P0 = 17;
const static size_t RAW_FRONT_LEFT_CROTCH_P1 = 18;
const static size_t RAW_FRONT_LEFT_KNEE_P0 = 19;
const static size_t RAW_FRONT_LEFT_KNEE_P1 = 20;
const static size_t RAW_FRONT_LEFT_WHEEL = 21;
const static size_t RAW_REAR_LEFT_CROTCH_Y = 22;
const static size_t RAW_REAR_LEFT_CROTCH_P0 = 23;
const static size_t RAW_REAR_LEFT_CROTCH_P1 = 24;
const static size_t RAW_REAR_LEFT_KNEE_P0 = 25;
const static size_t RAW_REAR_LEFT_KNEE_P1 = 26;
const static size_t RAW_REAR_LEFT_WHEEL = 27;
// 28 - 31: N/A
// 32 - 34: IMU (uint8_t * 6)

// sensor index (as int8_t)
// UPPER:
const static size_t RIGHT_HAND_SENSOR_FX = 30;
const static size_t RIGHT_HAND_SENSOR_FY = 31;
const static size_t RIGHT_HAND_SENSOR_FZ = 32;
const static size_t RIGHT_HAND_SENSOR_RX = 33;
const static size_t RIGHT_HAND_SENSOR_RY = 34;
const static size_t RIGHT_HAND_SENSOR_RZ = 35;

const static size_t LEFT_HAND_SENSOR_FX = 62;
const static size_t LEFT_HAND_SENSOR_FY = 63;
const static size_t LEFT_HAND_SENSOR_FZ = 64;
const static size_t LEFT_HAND_SENSOR_RX = 65;
const static size_t LEFT_HAND_SENSOR_RY = 66;
const static size_t LEFT_HAND_SENSOR_RZ = 67;

// LOWER (4LEGS has only IMU):
const static size_t IMU_AX = 70;
const static size_t IMU_AY = 71;
const static size_t IMU_AZ = 72;
const static size_t IMU_GX = 73;
const static size_t IMU_GY = 74;
const static size_t IMU_GZ = 75;


// command list
const static uint8_t CMD_MOTOR_CUR = 0x20;
const static uint8_t CMD_MOTOR_ACC = 0x24;
const static uint8_t CMD_MOTOR_GAIN = 0x25;
const static uint8_t CMD_GET_POS = 0x42;
const static uint8_t CMD_GET_TMP = 0x43;
const static uint8_t CMD_GET_CUR = 0x45;
const static uint8_t CMD_MOTOR_SRV = 0x50;
const static uint8_t CMD_MOVE_INC = 0x67;
const static uint8_t CMD_MOVE_ABS = 0x68;
}

class AJointIndex {
 public:
  size_t id;
  size_t stroke_index;
  size_t raw_index;

  AJointIndex(size_t i, size_t sidx, size_t ridx) :
      id(i), stroke_index(sidx), raw_index(ridx) {
  }
  AJointIndex(const AJointIndex& aji) {
    id = aji.id;
    stroke_index = aji.stroke_index;
    raw_index = aji.raw_index;
  }
  AJointIndex& operator=(const AJointIndex& aji) {
    id = aji.id;
    stroke_index = aji.stroke_index;
    raw_index = aji.raw_index;
    return *this;
  }
};

class AeroController {
 public:
  AeroController(io_service& ios, std::string& port);
  ~AeroController();

  // basic commands
  void seed_485_send(std::vector<uint8_t>& send_data);
  void seed_485_read(std::vector<uint8_t>& read_data);
  void flush();

  void set_command_header(uint8_t id, uint8_t cmd, uint16_t time,
                          std::vector<uint8_t>& dat);
  void set_check_sum(std::vector<uint8_t>& dat);

  // servo
  void servo_command(int16_t d0);
  void servo_on();
  void servo_off();

  void set_position(std::vector<int16_t>& stroke_vector, uint16_t time);
  void get_position(std::vector<int16_t>& stroke_vector);

  bool verbose() {return verbose_;}
  void verbose(bool v) {verbose_ = v;}

 private:
  io_service& io_;
  serial_port ser_;
  bool verbose_;

  std::vector<int16_t> stroke_vector_;
  std::vector<int16_t> stroke_ref_vector_;
  std::vector<int16_t> stroke_cur_vector_;

  std::vector<AJointIndex> joint_indices_;

  int16_t decode_short_(uint8_t* raw);
  void encode_short_(int16_t value, uint8_t* raw);

  /// @brief stroke_vector (int16_t) to raw_vector(uint8_t)
  void stroke_to_raw_(std::vector<int16_t>& stroke, std::vector<uint8_t>& raw);

  /// @brief raw_vector(uint8_t) to stroke_vector (int16_t)
  void raw_to_stroke_(std::vector<uint8_t>& raw, std::vector<int16_t>& stroke);
};


#endif  // HRPSYS_AERO_BRIDGE_AERO_CONTROLLER_HPP_
