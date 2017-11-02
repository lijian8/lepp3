/*
  @file iface_ec_lola.hpp

  Ethercat device control ids

  Copyright  Institute of Applied Mechanics, TU-Muenchen
  All rights reserved.
*/

#ifndef __IFACE_EC_LOLA_HPP__
#define __IFACE_EC_LOLA_HPP__

#ifdef QNX
#include <devctl.h>
#include <stdint.h>

//!device name for resource manager
#define EC_DEVNAME "/dev/lola/ec"

//!shm address (target data to elmos)
const std::string target_shm ="/am2b_hwl_iport";
//!shm address (sensor data)
const std::string sensor_shm ="/am2b_hwl_oport";


#define EC_CMD_CODE 1
//!goto "idle" mode 
#define DCMD_EC_GOTO_IDLE        __DION(_DCMD_MISC, EC_CMD_CODE+(uint8_t)ec_state_t::EC_STATE_IDLE)
//!goto "homing" mode           
#define DCMD_EC_GOTO_HOMING      __DION(_DCMD_MISC, EC_CMD_CODE+(uint8_t)ec_state_t::EC_STATE_HOMING)
//!goto "position control" mode (on elmo)
#define DCMD_EC_GOTO_ELMO_PCONT __DION(_DCMD_MISC, EC_CMD_CODE+(uint8_t)ec_state_t::EC_STATE_PCONT)
//!goto fault mode              
#define DCMD_EC_GOTO_FAULT      __DION(_DCMD_MISC, EC_CMD_CODE+(uint8_t)ec_state_t::EC_STATE_FAULT)
//!goto fault mode
#define DCMD_EC_GOTO_ELMO_OPT   __DION(_DCMD_MISC, EC_CMD_CODE+(uint8_t)ec_state_t::EC_STATE_OPT)
//!reset force/torque sensors   
#define DCMD_EC_RESET_FTS       __DION(_DCMD_MISC, EC_CMD_CODE+(uint8_t)ec_state_t::EC_STATE_RESET_FTS)
//!optimize ad-baudrate (for max freq.)  
#define DCMD_EC_TUNE_AD_FTS     __DION(_DCMD_MISC, EC_CMD_CODE+(uint8_t)ec_state_t::EC_STATE_TUNE_AD_FTS)
//!reset inertial measurement unsensors
#define DCMD_EC_RESET_IMU       __DION(_DCMD_MISC, EC_CMD_CODE+(uint8_t)ec_state_t::EC_STATE_RESET_IMU)
//!reset error on all slaves    
#define DCMD_EC_RESET_ERROR     __DION(_DCMD_MISC, EC_CMD_CODE+(uint8_t)ec_state_t::EC_STATE_RESET_ERROR)

//!start log
#define DCMD_EC_START_LOG          __DION(_DCMD_MISC, EC_CMD_CODE+10)
//!stop log
#define DCMD_EC_STOP_LOG           __DION(_DCMD_MISC, EC_CMD_CODE+11)
//!dump log
#define DCMD_EC_DUMP_LOG           __DION(_DCMD_MISC, EC_CMD_CODE+12)

#define DCMD_EC_SET_IMU_OFFSET_PHI0    __DIOT(_DCMD_MISC, EC_CMD_CODE+20, double)
#define DCMD_EC_SET_IMU_OFFSET_PHI1    __DIOT(_DCMD_MISC, EC_CMD_CODE+21, double)
                                       
//!set elmo control gains              
#define DCMD_EC_SET_ELMO_PCONT_KP      __DIOT(_DCMD_MISC, EC_CMD_CODE+22, EventIntFlt::Data)
#define DCMD_EC_SET_ELMO_VCONT_KP      __DIOT(_DCMD_MISC, EC_CMD_CODE+23, EventIntFlt::Data)
#define DCMD_EC_SET_ELMO_VCONT_KI      __DIOT(_DCMD_MISC, EC_CMD_CODE+24, EventIntFlt::Data)
#define DCMD_EC_SET_ELMO_ICONT_KP      __DIOT(_DCMD_MISC, EC_CMD_CODE+25, EventIntFlt::Data)
#define DCMD_EC_SET_ELMO_ICONT_KI      __DIOT(_DCMD_MISC, EC_CMD_CODE+26, EventIntFlt::Data)
#define DCMD_EC_SET_ELMO_ICONT_FF      __DIOT(_DCMD_MISC, EC_CMD_CODE+27, EventIntFlt::Data)


//!get program info
#define DCMD_EC_GET_HWL_INFO                   __DIOF(_DCMD_MISC, EC_CMD_CODE+30, HWLInfo)


#endif //QNX

/*! States */
enum class ec_state_t
{
  EC_STATE_NULL = 0,
  EC_STATE_IDLE = 1,
  EC_STATE_HOMING = 2,
  EC_STATE_PCONT  = 3,
  EC_STATE_FAULT  = 4,
  EC_STATE_OPT   = 5,
  EC_STATE_RESET_FTS = 6,
  EC_STATE_TUNE_AD_FTS = 7,
  EC_STATE_RESET_IMU = 8,
  EC_STATE_RESET_ERROR = 9,
  EC_STATE_CALIB = 10
};
inline std::string getECStateStr(const ec_state_t& state) 
{
  switch(state) 
    {
    case ec_state_t::EC_STATE_NULL:
      return std::string("EC_STATE_NULL");

    case ec_state_t::EC_STATE_IDLE:
      return std::string("EC_STATE_IDLE");

    case ec_state_t::EC_STATE_HOMING:
      return std::string("EC_STATE_HOMING");

    case ec_state_t::EC_STATE_PCONT:
      return std::string("EC_STATE_PCONT");

    case ec_state_t::EC_STATE_FAULT:
      return std::string("EC_STATE_FAULT");

  case ec_state_t::EC_STATE_OPT:
    return std::string("EC_STATE_OPT");

    case ec_state_t::EC_STATE_RESET_FTS:
      return std::string("EC_STATE_RESET_FTS");

    case ec_state_t::EC_STATE_TUNE_AD_FTS:
      return std::string("EC_STATE_TUNE_AD_FTS");

    case ec_state_t::EC_STATE_RESET_IMU:
      return std::string("EC_STATE_RESET_IMU");

    case ec_state_t::EC_STATE_RESET_ERROR:
      return std::string("EC_STATE_RESET_ERROR");

    case ec_state_t::EC_STATE_CALIB:
      return std::string("EC_STATE_CALIB");
    }
}



//! Elmo states
/* The assigned integer value matches the importance of the states for
   comparison. (lower = more important)
 */
enum class ElmoState : uint16_t {
  
  /*! fault occured - depending on error handling the high voltage may be off */
  FAULT = 0,
  
  /*! INIT - This is the initial state before any state is read from the device - high voltage should be off! */
  INIT = 1,

  /*! idle state, which is restored after a fault reset or power up - high voltage should be off! */
  IDLE = 8,

  /*! Elmo in "On" State, which means the power amplifier is working, but the drive mode is still disabled */
  READY = 7,
  
  /*! Motor without hall sensors perform the commutation or alignment in this state */
  COMMUTATION = 2,

  /* Drive module activated, commands are executed */
  OPERATIONAL = 10,

  // ------------------------------------------------------------------------
  // The following states are special states for homing. They basically represent one of the 3 main states 
  // with additional state information about successful homing / and the state of the drive
  // ------------------------------------------------------------------------

  /*! In homing IDLE state - basically IDLE with activated homing mode.
    This is an (internal) intermediate state to reflect the Elmo modes */
  HOMING_IDLE = 3,

  /*! In homing READY state - Basically READY with activated homing mode. 
     This mode is an intermediate state before switching to HOMING_OPERATIONAL.*/
  HOMING_READY = 4,

  /*! In homing state. This is basically OPERATIONAL with the intention to achieve homing / alignment of the drive.
      During this first change to OPERATIONAL, the elmo might also execute the parametrized commutation search (alignment). The preHoming flag
      is set if this state is reached. Before actual homing command is executed, the user program has to call ackHoming() on the device
  */
  HOMING_OPERATIONAL = 5,
  
  /*! Homing occured, the joint moves to zero position before state changes to HOMED */
  HOMING_ZEROING = 6,

  /*! Homing acquired state. Elmo returns to this state, if homing is acquired. This corresponds to a READY state of the device */
  HOMED = 9
};

//! Function to get a string representation of an Elmo State
inline std::string convertElmoStateToStr(const ElmoState& state) {

  switch(state) {
  
    case ElmoState::FAULT:
    return std::string("FAULT");
    
    case ElmoState::INIT:
    return std::string("INIT");
  
    case ElmoState::IDLE:
    return std::string("IDLE");
  
    case ElmoState::READY:
    return std::string("READY");
    
    case ElmoState::COMMUTATION:
    return std::string("COMMUTATION");
        
    case ElmoState::OPERATIONAL:
    return std::string("OPERATIONAL");
  
    case ElmoState::HOMING_READY:
    return std::string("HOMING_READY");
  
    case ElmoState::HOMING_OPERATIONAL:
    return std::string("HOMING_OPERATIONAL");
    
    case ElmoState::HOMING_ZEROING:
    return std::string("HOMING_ZEROING");

    case ElmoState::HOMING_IDLE:
    return std::string("HOMING_IDLE");
  
    case ElmoState::HOMED:
    return std::string("HOMED");
  
    default:
    return std::string("UNKNOWN");
  }
}


#endif
