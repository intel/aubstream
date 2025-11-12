/*
 * Copyright (C) 2023-2025 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

DECLARE_SETTING_VARIABLE(bool, PrintSettings, false, "Print all settings to stdout")
DECLARE_SETTING_VARIABLE(int, ExeclistSubmitPortSubmission, -1, "Enable submission via ELSP")
DECLARE_SETTING_VARIABLE(int, TbxConnectionDelayInSeconds, -1, "-1: default. >=0: seconds to wait before initializing TBX connection")
DECLARE_SETTING_VARIABLE(int, LogLevel, 0, "Bitfield. 0: default - logs not printed. >=0: print logs of specific level")
DECLARE_SETTING_VARIABLE(int, IndirectRingState, -1, "Enable indirect ring state")
