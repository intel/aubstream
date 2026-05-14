/*
 * Copyright (C) 2023-2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

DECLARE_SETTING_VARIABLE(bool, PrintSettings, false, "Print all settings to stdout")
DECLARE_SETTING_VARIABLE(int, ExeclistSubmitPortSubmission, -1, "Enable submission via ELSP")
DECLARE_SETTING_VARIABLE(int, TbxConnectionDelayInSeconds, -1, "-1: default. >=0: seconds to wait before initializing TBX connection")
DECLARE_SETTING_VARIABLE(int, LogLevel, 0, "Bitfield. 0: default - logs not printed. >=0: print logs of specific level")
DECLARE_SETTING_VARIABLE(int, IndirectRingState, -1, "Enable indirect ring state")
DECLARE_SETTING_VARIABLE(bool, EnablePs64, true, "Enable set the PS64 bit for 16 consecutive 4KB pages")
DECLARE_SETTING_VARIABLE(bool, AppTransientForUncompressedCachedPages, false, "Enables the App-Transient PAT attribute for uncompressed cached pages.")
