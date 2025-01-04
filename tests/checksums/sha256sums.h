#pragma once

#include <string>
#include <unordered_map>

#include "fnv.h"
#include "fnv_compressed.h"
#include "fnv_shared.h"
#include "fo3.h"
#include "fo3_compressed.h"
#include "fo3_shared.h"
#include "fo4.h"
#include "fo4_compressed.h"
#include "fo4_shared.h"
#include "fo4vr.h"
#include "fo4vr_compressed.h"
#include "fo4vr_shared.h"
#include "sf.h"
#include "sf_compressed.h"
#include "sf_shared.h"
#include "skyrimvr.h"
#include "skyrimvr_compressed.h"
#include "skyrimvr_shared.h"
#include "sse.h"
#include "sse_compressed.h"
#include "sse_shared.h"
#include "tes3.h"
#include "tes3_compressed.h"
#include "tes3_shared.h"
#include "tes4.h"
#include "tes4_compressed.h"
#include "tes4_shared.h"
#include "tes5.h"
#include "tes5_compressed.h"
#include "tes5_shared.h"

// the checksums were created by extracting and repacking game files with xEdit
// they will be invalid if there has been a game update after 11.10.2024
// for every game the steam version with english language was used
namespace checksums {
static const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> sha256sums
    = {{"tes3", tes3},
       {"tes3_compressed", tes3_compressed},
       {"tes3_shared", tes3_shared},
       {"tes4", tes4},
       {"tes4_compressed", tes4_compressed},
       {"tes4_shared", tes4_shared},
       {"tes5", tes5},
       {"tes5_compressed", tes5_compressed},
       {"tes5_shared", tes5_shared},
       {"sse", sse},
       {"sse_compressed", sse_compressed},
       {"sse_shared", sse_shared},
       {"skyrimvr", skyrimvr},
       {"skyrimvr_compressed", skyrimvr_compressed},
       {"skyrimvr_shared", skyrimvr_shared},
       {"fo3", fo3},
       {"fo3_compressed", fo3_compressed},
       {"fo3_shared", fo3_shared},
       {"fnv", fnv},
       {"fnv_compressed", fnv_compressed},
       {"fnv_shared", fnv_shared},
       {"fo4", fo4},
       {"fo4_compressed", fo4_compressed},
       {"fo4_shared", fo4_shared},
       {"fo4vr", fo4vr},
       {"fo4vr_compressed", fo4vr_compressed},
       {"fo4vr_shared", fo4vr_shared},
       {"sf", sf},
       {"sf_compressed", sf_compressed},
       {"sf_shared", sf_shared}};

} // namespace checksums
