/*
 * Copyright (C) 2022 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

namespace aub_stream {
template <typename Type>
struct WhiteBox : public Type {
    using Type::Type;
};
} // namespace aub_stream