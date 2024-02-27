/*
 * Copyright (C) 2023-2024 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

DECLARE_SETTING_VARIABLE(bool, PrintSettings, false, "Print all settings to stdout")
DECLARE_SETTING_VARIABLE(int, ExeclistSubmitPortSubmission, -1, "Enable submission via ELSP")
DECLARE_SETTING_VARIABLE(int, TbxConnectionDelayInSeconds, -1, "-1: default. >=0: seconds to wait before initializing TBX connection")