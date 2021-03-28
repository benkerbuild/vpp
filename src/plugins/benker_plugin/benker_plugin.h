
/*
 * benker_plugin.h - skeleton vpp engine plug-in header file
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
#ifndef __included_benker_plugin_h__
#define __included_benker_plugin_h__

#include <vnet/vnet.h>
#include <vnet/ip/ip.h>
#include <vnet/ethernet/ethernet.h>

#include <vppinfra/hash.h>
#include <vppinfra/error.h>

#include <plugins/gtpu/gtpu.h>
#include <vnet/classify/policer_classify.h>

typedef struct {
    /* API message ID base */
    u16 msg_id_base;

    /* on/off switch for the periodic function */
    u8 periodic_timer_enabled;
    /* Node index, non-zero if the periodic process has been created */
    u32 periodic_node_index;

    /* convenience */
    vlib_main_t * vlib_main;
    vnet_main_t * vnet_main;
    ethernet_main_t * ethernet_main;

    /* custom variables */
    uword *output_infc_map;     // key is sw_if_index, value is <routine1_intfc>##<routine2_intfc> (32bit each)
    gtpu_main_t *gtpu_main;     // gtpu main
    policer_classify_main_t *pcm;

} benker_plugin_main_t;

extern benker_plugin_main_t benker_plugin_main;

extern vlib_node_registration_t benker_plugin_node;
extern vlib_node_registration_t benker_plugin_periodic_node;

/* Periodic function events */
#define BENKER_PLUGIN_EVENT1 1
#define BENKER_PLUGIN_EVENT2 2
#define BENKER_PLUGIN_EVENT_PERIODIC_ENABLE_DISABLE 3

void benker_plugin_create_periodic_process (benker_plugin_main_t *);

#endif /* __included_benker_plugin_h__ */

/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */

