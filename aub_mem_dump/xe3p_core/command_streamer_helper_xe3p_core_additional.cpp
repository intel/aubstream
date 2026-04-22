/*
 * Copyright (C) 2026 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#include "aub_mem_dump/xe3p_core/command_streamer_helper_xe3p_core.h"

namespace aub_stream {

template <typename Helper>
void CommandStreamerHelperXe3pCore<Helper>::appendAdditionalEngineMMIO(MMIOList & /*engineMMIO*/) const {
}

template void CommandStreamerHelperXe3pCore<CommandStreamerHelperRcs>::appendAdditionalEngineMMIO(MMIOList &) const;
template void CommandStreamerHelperXe3pCore<CommandStreamerHelperCccs>::appendAdditionalEngineMMIO(MMIOList &) const;
template void CommandStreamerHelperXe3pCore<CommandStreamerHelperBcs>::appendAdditionalEngineMMIO(MMIOList &) const;
template void CommandStreamerHelperXe3pCore<CommandStreamerHelperLinkBcs>::appendAdditionalEngineMMIO(MMIOList &) const;
template void CommandStreamerHelperXe3pCore<CommandStreamerHelperVcs>::appendAdditionalEngineMMIO(MMIOList &) const;
template void CommandStreamerHelperXe3pCore<CommandStreamerHelperVecs>::appendAdditionalEngineMMIO(MMIOList &) const;
template void CommandStreamerHelperXe3pCore<CommandStreamerHelperCcs>::appendAdditionalEngineMMIO(MMIOList &) const;

} // namespace aub_stream
