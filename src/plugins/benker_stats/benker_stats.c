/*
 * benker_stats.c - skeleton vpp engine plug-in
 *
 * Copyright (c) <current-year> <your-organization>
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vnet/vnet.h>
#include <vnet/plugin/plugin.h>
#include <benker_stats/benker_stats.h>

#include <vpp/app/version.h>
#include <stdbool.h>

#define REPLY_MSG_ID_BASE bmp->msg_id_base
#include <vlibapi/api_helper_macros.h>

benker_stats_main_t benker_stats_main;

static clib_error_t * benker_stats_init (vlib_main_t * vm)
{
  benker_stats_main_t * bmp = &benker_stats_main;
  clib_error_t * error = 0;

  bmp->vlib_main = vm;
  bmp->vnet_main = vnet_get_main();

  return error;
}

VLIB_INIT_FUNCTION (benker_stats_init);

/* *INDENT-OFF* */
VNET_FEATURE_INIT (benker_stats, static) =
{
  .arc_name = "device-input",
  .node_name = "benker_stats",
  .runs_before = VNET_FEATURES ("benker_plugin"),
};
/* *INDENT-ON */

/* *INDENT-OFF* */
VLIB_PLUGIN_REGISTER () =
{
  .version = VPP_BUILD_VER,
  .description = "benker_stats plugin for learning vpp",
};
/* *INDENT-ON* */

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */
