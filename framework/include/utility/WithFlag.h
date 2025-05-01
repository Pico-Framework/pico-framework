/**
 * @file WithFlag.h
 * @author Ian Archbell
 * @brief RAII-style scoped flag setter. Temporarily sets a boolean flag and restores it on destruction.
 *
 * Useful for suppressing behavior or toggling internal state temporarily.
 * Pattern: WithFlag flagGuard(myFlag); // sets myFlag = true, restores original value on scope exit.
 *
 * @version 0.1
 * @date 2025-05-01
 * @license MIT License
 * @copyright Copyright (c) 2025, Ian Archbell
 */

 #pragma once

 /**
  * @class WithFlag
  * @brief Temporarily sets a boolean flag and restores it automatically.
  */
 class WithFlag {
     bool& flag;
     bool previousValue;
 
 public:
     /**
      * @brief Construct the WithFlag and set the flag to the desired value (default true).
      * @param flag Reference to the flag to modify.
      * @param value The value to temporarily set (default: true).
      */
     explicit WithFlag(bool& flag, bool value = true)
         : flag(flag), previousValue(flag) {
         flag = value;
     }
 
     /**
      * @brief Destructor restores the original value of the flag.
      */
     ~WithFlag() {
         flag = previousValue;
     }
 };
 