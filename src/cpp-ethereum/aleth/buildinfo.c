/* Cable: CMake Bootstrap Library.
 * Copyright 2018 Pawel Bylica.
 * Licensed under the Apache License, Version 2.0. See the LICENSE file.
 */

#include "buildinfo.h"

const struct buildinfo* aleth_get_buildinfo()
{
    static const struct buildinfo buildinfo = {
        .project_name = "aleth",
        .project_version = "1.8.0",
        .project_name_with_version = "aleth-1.8.0",
        .git_commit_hash = "af7b28a066b536010ef9f5da5ce2321421b80176",
        .system_name = "",
        .system_processor = "",
        .compiler_id = "",
        .compiler_version = "",
        .build_type = "",
    };
    return &buildinfo;
}
