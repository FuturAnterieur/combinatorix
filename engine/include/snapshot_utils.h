#pragma once

#include "engine_export.h"
#include "logistics/include/attributes_info.h"

engine_API bool get_value_for_status(const attributes_info_snapshot &snapshot, entt::id_type hash);
engine_API parameter get_value_for_parameter(const attributes_info_snapshot &snapshot, entt::id_type hash);