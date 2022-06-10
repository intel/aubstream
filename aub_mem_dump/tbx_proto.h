/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once
#include <cstdint>

enum mem_types : uint32_t {
    MEM_TYPE_SYSTEM = 0,
    MEM_TYPE_LOCALMEM = 1,
    MEM_TYPE_MAX = 4
};

enum HAS_MSG_TYPE {
    HAS_MMIO_REQ_TYPE = 0,
    HAS_MMIO_RES_TYPE = 1,
    HAS_GTT_REQ_TYPE = 2,
    HAS_GTT_RES_TYPE = 3,
    HAS_WRITE_DATA_REQ_TYPE = 4,
    HAS_READ_DATA_REQ_TYPE = 5,
    HAS_READ_DATA_RES_TYPE = 6,
    HAS_MARKER_REQ_TYPE = 7,
    HAS_MARKER_RES_TYPE = 8,
    HAS_REPORT_REND_END_REQ_TYPE = 9,
    HAS_REPORT_REND_END_RES_TYPE = 10,
    HAS_CONTROL_REQ_TYPE = 11,
    HAS_PARAMS_REQ_TYPE = 12,
    HAS_PARAMS_RES_TYPE = 13,
    HAS_PCICFG_REQ_TYPE = 14,
    HAS_PCICFG_RES_TYPE = 15,
    HAS_GTT_PARAMS_REQ_TYPE = 16,
    HAS_EVENT_REQ_TYPE = 17,
    HAS_INNER_VAR_REQ_TYPE = 18,
    HAS_INNER_VAR_RES_TYPE = 19,
    HAS_INNER_VAR_LIST_REQ_TYPE = 20,
    HAS_INNER_VAR_LIST_RES_TYPE = 21,
    HAS_FUNNY_IO_REQ_TYPE = 22,
    HAS_FUNNY_IO_RES_TYPE = 23,
    HAS_IO_REQ_TYPE = 24,
    HAS_IO_RES_TYPE = 25,
    HAS_RPC_REQ_TYPE = 26,
    HAS_RPC_RES_TYPE = 27,
    HAS_CL_FLUSH_REQ_TYPE = 28,
    HAS_CL_FLUSH_RES_TYPE = 29,
    HAS_SYNC_ALL_PAGES_REQ_TYPE = 30,
    HAS_SYNC_ALL_PAGES_RES_TYPE = 31,
    HAS_GD2_MESSAGE_TYPE = 32,
    HAS_SIMTIME_RES_TYPE = 33,
    HAS_RL_STATUS_RES_TYPE = 34,
    HAS_LOCK_PAGE_REQ_TYPE = 35,
    HAS_UNLOCK_PAGES_REQ_TYPE = 36,
    HAS_SET_UNTILING_REORDER_REQ_TYPE = 37,
    HAS_RESET_UNTILING_REORDER_REQ_TYPE = 38,
    HAS_MMIO2_REQ_TYPE = 39,
    HAS_MMIO2_RES_TYPE = 40,
    HAS_IOSF_SB_DATA_REQ_TYPE = 41,
    HAS_IOSF_SB_DATA_RES_TYPE = 42,
    HAS_COMPARE_DATA_REQ_TYPE = 43,
    HAS_COMPARE_DATA_RES_TYPE = 44,
    NUM_OF_MSG_TYPE
};

struct HAS_HDR {
    union {
        uint32_t msg_type;
        HAS_MSG_TYPE type;
    };
    uint32_t trans_id;
    uint32_t size;
};

enum {
    MSG_TYPE_MMIO = 0,
    MSG_TYPE_IO,
    MSG_TYPE_FUNNY_IO
};

struct HAS_MMIO_REQ {
    uint32_t write : 1;
    uint32_t size : 3;
    uint32_t dev_idx : 2;
    uint32_t msg_type : 3;
    uint32_t reserved : 7;
    uint32_t delay : 16;

    uint32_t offset;
    uint32_t data;

    enum {
        HAS_MSG_TYPE = HAS_MMIO_REQ_TYPE
    };
};

struct HAS_MMIO_EXT_REQ {
    struct HAS_MMIO_REQ mmio_req;
    uint32_t sourceid : 8;
    uint32_t reserved1 : 24;
    enum {
        HAS_MSG_TYPE = HAS_MMIO_REQ_TYPE
    };
};

struct HAS_MMIO_RES {
    uint32_t data;

    enum {
        HAS_MSG_TYPE = HAS_MMIO_RES_TYPE
    };
};

struct HAS_GTT32_REQ {
    uint32_t write : 1;
    uint32_t reserved : 31;

    uint32_t offset;
    uint32_t data;

    enum {
        HAS_MSG_TYPE = HAS_GTT_REQ_TYPE
    };
};

struct HAS_GTT32_RES {
    uint32_t data;

    enum {
        HAS_MSG_TYPE = HAS_GTT_RES_TYPE
    };
};

struct HAS_GTT64_REQ {
    uint32_t write : 1;
    uint32_t reserved : 31;

    uint32_t offset;
    uint32_t data;
    uint32_t data_h;

    enum {
        HAS_MSG_TYPE = HAS_GTT_REQ_TYPE
    };
};

struct HAS_GTT64_RES {
    uint32_t data;
    uint32_t data_h;

    enum {
        HAS_MSG_TYPE = HAS_GTT_RES_TYPE
    };
};

struct HAS_WRITE_DATA_REQ {
    uint32_t addr_type : 1;
    uint32_t mask_exist : 1;
    uint32_t frontdoor : 1;
    uint32_t take_ownership : 1;
    uint32_t model_owned : 1;
    uint32_t cacheline_disable : 1;
    uint32_t memory_type : 2;
    uint32_t reserved : 16;
    uint32_t address_h : 8;
    uint32_t address;
    uint32_t size;

    enum {
        HAS_MSG_TYPE = HAS_WRITE_DATA_REQ_TYPE
    };
};

/**
*  Extended version of HAS_WRITE_DATA_REQ
*/
struct HAS_WRITE_DATA_EXT_REQ {
    /**
   * Original format of HAS_WRITE_DATA_REQ.
   */
    struct HAS_WRITE_DATA_REQ write_req;
    /**
   * MSB bits of address.
   */
    uint32_t address_h;

    enum {
        HAS_MSG_TYPE = HAS_WRITE_DATA_REQ_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_READ_DATA_REQ {
    uint32_t addr_type : 1;
    uint32_t frontdoor : 1;
    uint32_t ownership_req : 1;
    uint32_t model_owned : 1;
    uint32_t cacheline_disable : 1;
    uint32_t memory_type : 2;
    uint32_t reserved : 17;
    uint32_t address_h : 8;
    uint32_t address;
    uint32_t size;

    enum {
        HAS_MSG_TYPE = HAS_READ_DATA_REQ_TYPE
    };
};

/**
 *
 *  Extended version of HAS_READ_DATA_REQ
 */
struct HAS_READ_DATA_EXT_REQ {
    /**
   * Original format of HAS_READ_DATA_REQ.
   */
    struct HAS_READ_DATA_REQ read_req;
    /**
   * MSB bits of address.
   */
    uint32_t address_h;

    enum {
        HAS_MSG_TYPE = HAS_READ_DATA_REQ_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_READ_DATA_RES {
    uint32_t addr_type : 1;
    uint32_t mask_exist : 1;
    uint32_t last_page : 1;
    uint32_t ownership_res : 1;
    uint32_t reserved : 20;
    uint32_t address_h : 8;
    uint32_t address;
    uint32_t size;

    enum {
        HAS_MSG_TYPE = HAS_READ_DATA_RES_TYPE
    };
};

/**
 * Response packet for HAS_READ_DATA_EXT_REQ.
 */
struct HAS_READ_DATA_EXT_RES {
    /**
     * Original format of HAS_READ_DATA_RES.
     */
    struct HAS_READ_DATA_RES read_res;

    /**
     * MSB bits of address.
     */

    uint32_t address_h;

    enum {
        HAS_MSG_TYPE = HAS_READ_DATA_RES_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_CONTROL_REQ {
    uint32_t reset : 1;          // [0:0]
    uint32_t has : 1;            // [1:1]
    uint32_t rd_on_demand : 1;   // [2:2]
    uint32_t write_mask : 1;     // [3:3]
    uint32_t time_adv : 1;       // [4:4]
    uint32_t async_msg : 1;      // [5:5]
    uint32_t quit : 1;           // [6:6]
    uint32_t cncry_enb : 1;      // [7:7]
    uint32_t stime_enb : 1;      // [8:8]
    uint32_t full_reset : 1;     // [9:9]
    uint32_t auto_ownership : 1; // [10:10]
    uint32_t backdoor_model : 1; // [11:11]
    uint32_t flush : 1;          // [12:12]
    uint32_t reserved : 3;       // [15:13]

    uint32_t reset_mask : 1;          // [16:16]
    uint32_t has_mask : 1;            // [17:17]
    uint32_t rd_on_demand_mask : 1;   // [18:18]
    uint32_t write_mask_mask : 1;     // [19:19]
    uint32_t time_adv_mask : 1;       // [20:20]
    uint32_t async_msg_mask : 1;      // [21:21]
    uint32_t quit_mask : 1;           // [22:22]
    uint32_t cncry_enb_mask : 1;      // [23:23]
    uint32_t stime_enb_mask : 1;      // [24:24]
    uint32_t full_reset_mask : 1;     // [25:25]
    uint32_t auto_ownership_mask : 1; // [26:26]
    uint32_t backdoor_model_mask : 1; // [27:27]
    uint32_t flush_mask : 1;          // [28:28]
    uint32_t reserved_mask : 3;       // [31:29]

    enum {
        HAS_MSG_TYPE = HAS_CONTROL_REQ_TYPE
    };
};

struct HAS_REPORT_REND_END_REQ {
    uint32_t timeout;

    enum {
        HAS_MSG_TYPE = HAS_REPORT_REND_END_REQ_TYPE
    };
};

struct HAS_REPORT_REND_END_RES {
    uint32_t timeout : 1;
    uint32_t reserved : 31;

    enum {
        HAS_MSG_TYPE = HAS_REPORT_REND_END_RES_TYPE
    };
};

struct HAS_PCICFG_REQ {
    uint32_t write : 1;
    uint32_t size : 3;
    uint32_t bus : 8;
    uint32_t device : 5;
    uint32_t function : 3;
    uint32_t reserved : 12;
    uint32_t offset;
    uint32_t data;

    enum {
        HAS_MSG_TYPE = HAS_PCICFG_REQ_TYPE
    };
};

struct HAS_PCICFG_RES {
    uint32_t data;
};

struct HAS_GTT_PARAMS_REQ {
    uint32_t base;
    uint32_t base_h : 8;
    uint32_t size : 24;

    enum {
        HAS_MSG_TYPE = HAS_GTT_PARAMS_REQ_TYPE
    };
};

struct HAS_EVENT_OBSOLETE_REQ {
    uint32_t offset;
    uint32_t data;

    enum {
        HAS_MSG_TYPE = HAS_EVENT_REQ_TYPE
    };
};

struct HAS_EVENT_REQ {
    uint32_t offset;
    uint32_t data;
    uint32_t dev_idx : 2;
    uint32_t reserved : 30;

    enum {
        HAS_MSG_TYPE = HAS_EVENT_REQ_TYPE
    };
};

struct HAS_INNER_VAR_REQ {
    uint32_t write : 1;
    uint32_t non_dword : 16;
    uint32_t reserved : 15;
    uint32_t id;
    uint32_t data;

    enum {
        HAS_MSG_TYPE = HAS_INNER_VAR_REQ_TYPE
    };
};

struct HAS_INNER_VAR_RES {
    uint32_t data;

    enum {
        HAS_MSG_TYPE = HAS_INNER_VAR_RES_TYPE
    };
};

struct HAS_INNER_VAR_LIST_RES {
    uint32_t size;

    enum {
        HAS_MSG_TYPE = HAS_INNER_VAR_LIST_RES_TYPE
    };
};

struct HAS_INTERNAL_VAR_LIST_ENTRY_RES {
    uint32_t id;
    uint32_t min;
    uint32_t max;
    uint32_t desc_size;
};

struct HAS_FUNNY_IO_REQ {
    uint32_t write : 1;
    uint32_t reserved : 28;
    uint32_t size : 3;
    uint32_t offset;
    uint32_t value;

    enum {
        HAS_MSG_TYPE = HAS_FUNNY_IO_REQ_TYPE
    };
};

struct HAS_FUNNY_IO_RES {
    uint32_t data;

    enum {
        HAS_MSG_TYPE = HAS_FUNNY_IO_RES_TYPE
    };
};

struct HAS_IO_REQ {
    uint32_t write : 1;
    uint32_t dev_idx : 2;
    uint32_t reserved : 26;
    uint32_t size : 3;
    uint32_t offset;
    uint32_t value;

    enum {
        HAS_MSG_TYPE = HAS_IO_REQ_TYPE
    };
};

struct HAS_IO_RES {
    uint32_t data;

    enum {
        HAS_MSG_TYPE = HAS_IO_RES_TYPE
    };
};

struct HAS_RPC_REQ {
    uint32_t size;

    enum {
        HAS_MSG_TYPE = HAS_RPC_REQ_TYPE
    };
};

struct HAS_RPC_RES {
    uint32_t status;
    uint32_t size;

    enum {
        HAS_MSG_TYPE = HAS_RPC_RES_TYPE
    };
};

struct HAS_CL_FLUSH_REQ {
    uint32_t reserved : 23;
    uint32_t ignore : 1;
    uint32_t address_h : 8;
    uint32_t address;
    uint32_t size;
    uint32_t delay;

    enum {
        HAS_MSG_TYPE = HAS_CL_FLUSH_REQ_TYPE
    };
};

struct HAS_CL_FLUSH_RES {
    uint32_t data;

    enum {
        HAS_MSG_TYPE = HAS_CL_FLUSH_RES_TYPE
    };
};

struct HAS_SIMTIME_RES {
    uint32_t data_l;
    uint32_t data_h;

    enum {
        HAS_MSG_TYPE = HAS_SIMTIME_RES_TYPE
    };
};

struct HAS_GD2_MESSAGE {
    uint32_t subOpcode;
    uint32_t data[1];

    enum {
        HAS_MSG_TYPE = HAS_GD2_MESSAGE_TYPE
    };
};

struct HAS_RL_STATUS_RES {
    uint32_t ringOffset;
    uint32_t reserved;
    enum {
        HAS_MSG_TYPE = HAS_RL_STATUS_RES_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_LOCK_PAGE_REQ {
    uint32_t reserved : 24;
    uint32_t address_h : 8;
    uint32_t address;
    enum {
        HAS_MSG_TYPE = HAS_LOCK_PAGE_REQ_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_UNLOCK_PAGES_REQ {
    enum {
        HAS_MSG_TYPE = HAS_UNLOCK_PAGES_REQ_TYPE,
        HAS_IS_EMPTY_MSG = true
    };
};

struct HAS_SET_UNTILING_REORDER_REQ {
    uint32_t start_gfx_page;
    uint32_t end_gfx_page;
    uint32_t offset;
    uint32_t pitch;
    uint32_t reserved : 21;
    uint32_t id : 8;
    uint32_t type : 3;
    enum {
        HAS_MSG_TYPE = HAS_SET_UNTILING_REORDER_REQ_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_RESET_UNTILING_REORDER_REQ {
    uint32_t reserved : 24;
    uint32_t id : 8;
    enum {
        HAS_MSG_TYPE = HAS_RESET_UNTILING_REORDER_REQ_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_MMIO2_REQ {
    uint32_t write : 1;
    uint32_t size : 5;
    uint32_t dev_idx : 2;
    uint32_t msg_type : 3;
    uint32_t function : 3;
    uint32_t origin : 1;
    uint32_t reserved : 1;
    uint32_t delay : 16;
    uint32_t offset;
    uint32_t data;
    uint32_t data_h;
    enum {
        HAS_MSG_TYPE = HAS_MMIO2_REQ_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_MMIO2_EXT_REQ {
    struct HAS_MMIO2_REQ mmio_req;
    uint32_t sourceid : 8;
    uint32_t function : 6;
    uint32_t reserved1 : 18;
    enum {
        HAS_MSG_TYPE = HAS_MMIO2_REQ_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_MMIO2_RES {
    uint32_t data;
    uint32_t data_h;
    enum {
        HAS_MSG_TYPE = HAS_MMIO2_RES_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_IOSF_SB_DATA_REQ {
    uint32_t msg_type : 32;
    uint32_t src_id : 8;
    uint32_t dest_id : 8;
    uint32_t opcode : 8;
    uint32_t tag : 3;
    uint32_t eh : 1;
    uint32_t data_h;
    uint32_t data;
    uint32_t size;
    enum {
        HAS_MSG_TYPE = HAS_IOSF_SB_DATA_REQ_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_IOSF_SB_DATA_RES {
    uint32_t msg_type : 32;
    uint32_t src_id : 8;
    uint32_t dest_id : 8;
    uint32_t opcode : 8;
    uint32_t tag : 3;
    uint32_t eh : 1;
    uint32_t data_h;
    uint32_t data;
    uint32_t size;
    enum {
        HAS_MSG_TYPE = HAS_IOSF_SB_DATA_RES_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_COMPARE_DATA_REQ {
    uint32_t mmio : 1;
    uint32_t frontdoor : 1;
    uint32_t operation : 3;
    uint32_t timeout : 12;
    uint32_t reserved : 7;
    uint32_t address_h : 8;
    uint32_t address;
    uint32_t size;
    enum {
        HAS_MSG_TYPE = HAS_COMPARE_DATA_REQ_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_COMPARE_DATA_RES {
    uint32_t result : 1;
    uint32_t reserved : 31;
    enum {
        HAS_MSG_TYPE = HAS_COMPARE_DATA_RES_TYPE,
        HAS_IS_EMPTY_MSG = false
    };
};

struct HAS_SYNC_ALL_PAGES_REQ {
    enum {
        HAS_MSG_TYPE = HAS_SYNC_ALL_PAGES_REQ_TYPE,
        HAS_IS_EMPTY_MSG = true
    };
};

struct HAS_SYNC_ALL_PAGES_RES {
    enum {
        HAS_MSG_TYPE = HAS_SYNC_ALL_PAGES_RES_TYPE,
        HAS_IS_EMPTY_MSG = true
    };
};

union HAS_MSG_BODY {
    struct HAS_MMIO_REQ mmio_req;
    struct HAS_MMIO_EXT_REQ mmio_req_ext;
    struct HAS_MMIO_RES mmio_res;
    struct HAS_GTT32_REQ gtt32_req;
    struct HAS_GTT32_RES gtt32_res;
    struct HAS_GTT64_REQ gtt64_req;
    struct HAS_GTT64_RES gtt64_res;
    struct HAS_WRITE_DATA_REQ write_req;
    struct HAS_WRITE_DATA_EXT_REQ write_ext_req;
    struct HAS_READ_DATA_REQ read_req;
    struct HAS_READ_DATA_EXT_REQ read_ext_req;
    struct HAS_READ_DATA_RES read_res;
    struct HAS_READ_DATA_EXT_RES read_ext_res;
    struct HAS_CONTROL_REQ control_req;
    struct HAS_REPORT_REND_END_REQ render_req;
    struct HAS_REPORT_REND_END_RES render_res;
    struct HAS_PCICFG_REQ pcicfg_req;
    struct HAS_PCICFG_RES pcicfg_res;
    struct HAS_GTT_PARAMS_REQ gtt_params_req;
    struct HAS_EVENT_REQ event_req;
    struct HAS_EVENT_OBSOLETE_REQ event_obsolete_req;
    struct HAS_INNER_VAR_REQ inner_var_req;
    struct HAS_INNER_VAR_RES inner_var_res;
    struct HAS_INNER_VAR_LIST_RES inner_var_list_res;
    struct HAS_IO_REQ io_req;
    struct HAS_IO_RES io_res;
    struct HAS_RPC_REQ rpc_req;
    struct HAS_RPC_RES rpc_res;
    struct HAS_CL_FLUSH_REQ flush_req;
    struct HAS_CL_FLUSH_RES flush_res;
    struct HAS_SIMTIME_RES stime_res;
    struct HAS_GD2_MESSAGE gd2_message_req;
    struct HAS_RL_STATUS_RES rl_status_res;
    struct HAS_LOCK_PAGE_REQ lock_page_req;
    struct HAS_UNLOCK_PAGES_REQ unlock_pages_req;
    struct HAS_SET_UNTILING_REORDER_REQ set_untiling_reorder_req;
    struct HAS_RESET_UNTILING_REORDER_REQ reset_untiling_reorder_req;
    struct HAS_MMIO2_REQ mmio2_req;
    struct HAS_MMIO2_EXT_REQ mmio2_req_ext;
    struct HAS_MMIO2_RES mmio2_res;
    struct HAS_SYNC_ALL_PAGES_REQ sync_all_pages_req;
    struct HAS_SYNC_ALL_PAGES_RES sync_all_pages_res;
    struct HAS_IOSF_SB_DATA_REQ iosf_sb_data_req;
    struct HAS_IOSF_SB_DATA_RES iosf_sb_data_res;
    struct HAS_COMPARE_DATA_REQ compare_req;
    struct HAS_COMPARE_DATA_RES compare_res;
};

struct HAS_MSG {
    struct HAS_HDR hdr;
    union HAS_MSG_BODY u;
};
