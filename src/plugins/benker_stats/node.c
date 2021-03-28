/*
 * node.c - skeleton vpp engine plug-in dual-loop node skeleton
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
#include <vlib/vlib.h>
#include <vnet/vnet.h>
#include <vnet/pg/pg.h>
#include <vppinfra/error.h>
#include <benker_stats/benker_stats.h>

#include <plugins/gtpu/gtpu.h>

typedef struct
{
  u32 sw_if_index;
  u32 next_index;
} benker_stats_trace_t;

#ifndef CLIB_MARCH_VARIANT

/* packet trace format function */
static u8 * format_benker_stats_trace (u8 * s, va_list * args)
{
  CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
  CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
  benker_stats_trace_t * t = va_arg (*args, benker_stats_trace_t *);
  
  s = format (s, "BENKER_STATS: sw_if_index %d, next index %d\n",
              t->sw_if_index, t->next_index);
  return s;
}

vlib_node_registration_t benker_stats_node;

#endif /* CLIB_MARCH_VARIANT */

#define foreach_benker_stats_error \
_(HANDLED, "Benker stats handled packets") \
_(NO_TUNNEL, "Benker stats no such tunnel (dropped)")

typedef enum 
{
#define _(sym,str) BENKER_STATS_ERROR_##sym,
  foreach_benker_stats_error
#undef _
  BENKER_STATS_N_ERROR,
} benker_stats_error_t;

#ifndef CLIB_MARCH_VARIANT
static char * benker_stats_error_strings[] = 
{
#define _(sym,string) string,
  foreach_benker_stats_error
#undef _
};
#endif /* CLIB_MARCH_VARIANT */

typedef enum 
{
  BENKER_STATS_NEXT_DROP,
  BENKER_STATS_NEXT_INTERFACE_OUTPUT,
  BENKER_STATS_N_NEXT,
} benker_stats_next_t;

always_inline uword
benker_stats_inline (vlib_main_t * vm,
     		 vlib_node_runtime_t * node, vlib_frame_t * frame,
		 int is_ip4, int is_trace)
{
  benker_stats_main_t * bmp = &benker_stats_main;
  vnet_interface_main_t * im = &bmp->gtpu_main->vnet_main->interface_main;
  u32 thread_index = vlib_get_thread_index();
  u32 n_left_from, *from;
  vlib_buffer_t *bufs[VLIB_FRAME_SIZE], **b;
  u16 nexts[VLIB_FRAME_SIZE], *next;
  gtpu4_tunnel_key_t gtpu_key;
  u32 pkts_handled = 0;

  from = vlib_frame_vector_args (frame);
  n_left_from = frame->n_vectors;

  vlib_get_buffers (vm, from, bufs, n_left_from);
  b = bufs;
  next = nexts;

  while (n_left_from >= 4 && false /* focus on 1 pkt handling first */)
    {
      /* Prefetch next iteration. */
      if (PREDICT_TRUE (n_left_from >= 8))
	{
	  vlib_prefetch_buffer_header (b[4], STORE);
	  vlib_prefetch_buffer_header (b[5], STORE);
	  vlib_prefetch_buffer_header (b[6], STORE);
	  vlib_prefetch_buffer_header (b[7], STORE);
          CLIB_PREFETCH (b[4]->data, CLIB_CACHE_LINE_BYTES, STORE);
          CLIB_PREFETCH (b[5]->data, CLIB_CACHE_LINE_BYTES, STORE);
          CLIB_PREFETCH (b[6]->data, CLIB_CACHE_LINE_BYTES, STORE);
          CLIB_PREFETCH (b[7]->data, CLIB_CACHE_LINE_BYTES, STORE);
	}

     /* $$$$ process 4x pkts right here */
      next[0] = 0;
      next[1] = 0;
      next[2] = 0;
      next[3] = 0;

      if (is_trace)
	{
	  if (b[0]->flags & VLIB_BUFFER_IS_TRACED)
	    {
	      benker_stats_trace_t *t = 
                   vlib_add_trace (vm, node, b[0], sizeof (*t));
	      t->next_index = next[0];
              t->sw_if_index = vnet_buffer(b[0])->sw_if_index[VLIB_RX];
	    }
	  if (b[1]->flags & VLIB_BUFFER_IS_TRACED)
	    {
	      benker_stats_trace_t *t = 
                    vlib_add_trace (vm, node, b[1], sizeof (*t));
	      t->next_index = next[1];
              t->sw_if_index = vnet_buffer(b[1])->sw_if_index[VLIB_RX];
	    }
	  if (b[2]->flags & VLIB_BUFFER_IS_TRACED)
	    {
	      benker_stats_trace_t *t = 
                    vlib_add_trace (vm, node, b[2], sizeof (*t));
	      t->next_index = next[2];
              t->sw_if_index = vnet_buffer(b[2])->sw_if_index[VLIB_RX];
	    }
	  if (b[3]->flags & VLIB_BUFFER_IS_TRACED)
	    {
	      benker_stats_trace_t *t = 
                    vlib_add_trace (vm, node, b[3], sizeof (*t));
	      t->next_index = next[3];
              t->sw_if_index = vnet_buffer(b[3])->sw_if_index[VLIB_RX];
	    }
	}

      b += 4;
      next += 4;
      n_left_from -= 4;
    }

  while (n_left_from > 0)
    {
      ip4_header_t *ip4_0;
      gtpu_header_t *gtpu4_0;
      gtpu_tunnel_t * gtpu_tunnel0;
      uword *tunnel0;
      u32 len0;

     /* $$$$ process 1 pkt right here */
      next[0] = BENKER_STATS_NEXT_INTERFACE_OUTPUT;

      /*
          main logic, pseudocode in python style

          key = getGtpuKey(pkts) # src.addr + teid
          tunnel = getGtpuTunnel(key)
          if not tunnel:
            drop(pkt)
            return
          validation(pkt) # may skip
          updateTunnelStats(pkt)

      */

      // offset (l2) 14-bytes => ip-header
      // offset (l3, udp) 20+8 = 28-bytes => gtpu-header
      ip4_0 = (void *)((u8*)vlib_buffer_get_current(b[0]) + 14);
      gtpu4_0 = (void *)((u8*)ip4_0 + 28);
      gtpu_key.src = ip4_0->src_address.as_u32;
      gtpu_key.teid = gtpu4_0->teid;
      tunnel0 = hash_get (bmp->gtpu_main->gtpu4_tunnel_by_key, gtpu_key.as_u64);

      if (PREDICT_FALSE (tunnel0 == NULL))
        {
          b[0]->error = node->errors[BENKER_STATS_ERROR_NO_TUNNEL];
          next[0] = BENKER_STATS_NEXT_DROP;
        }
      else
        {
          gtpu_tunnel0 = pool_elt_at_index (bmp->gtpu_main->tunnels, tunnel0[0]);
          len0 = gtpu4_0->length - ((gtpu4_0->ver_flags & GTPU_E_BIT) * 4) + ((gtpu4_0->ver_flags & GTPU_S_BIT) * 4); // buggy if more than 1 extension header
          vlib_increment_combined_counter 
            (im->combined_sw_if_counters + VNET_INTERFACE_COUNTER_RX,
             thread_index, gtpu_tunnel0->sw_if_index,
             1, len0);
        }

      if (is_trace)
	{
	  if (b[0]->flags & VLIB_BUFFER_IS_TRACED)
	    {
	      benker_stats_trace_t *t = 
                    vlib_add_trace (vm, node, b[0], sizeof (*t));
	      t->next_index = next[0];
              t->sw_if_index = vnet_buffer(b[0])->sw_if_index[VLIB_RX];
	    }
	}

      b += 1;
      next += 1;
      n_left_from -= 1;
      pkts_handled += 1;
    }

  vlib_buffer_enqueue_to_next (vm, node, from, nexts, frame->n_vectors);

  vlib_node_increment_counter (vm, benker_stats_node.index,
                               BENKER_STATS_ERROR_HANDLED, pkts_handled);

  return frame->n_vectors;
}

VLIB_NODE_FN (benker_stats_node) (vlib_main_t * vm, vlib_node_runtime_t * node,
                             vlib_frame_t * frame)
{
  if (PREDICT_FALSE (node->flags & VLIB_NODE_FLAG_TRACE))
    return benker_stats_inline (vm, node, frame, 1 /* is_ip4 */ ,
			    1 /* is_trace */ );
  else
    return benker_stats_inline (vm, node, frame, 1 /* is_ip4 */ ,
			    0 /* is_trace */ );
}

/* *INDENT-OFF* */
#ifndef CLIB_MARCH_VARIANT
VLIB_REGISTER_NODE (benker_stats_node) = 
{
  .name = "benker_stats",
  .vector_size = sizeof (u32),
  .format_trace = format_benker_stats_trace,
  .type = VLIB_NODE_TYPE_INTERNAL,
  
  .n_errors = ARRAY_LEN(benker_stats_error_strings),
  .error_strings = benker_stats_error_strings,

  .n_next_nodes = BENKER_STATS_N_NEXT,

  /* edit / add dispositions here */
  .next_nodes = {
        [BENKER_STATS_NEXT_DROP] = "error-drop",
        [BENKER_STATS_NEXT_INTERFACE_OUTPUT] = "interface-output",
  },
};
#endif /* CLIB_MARCH_VARIANT */
/* *INDENT-ON* */
/*
 * fd.io coding-style-patch-verification: ON
 *
 * Local Variables:
 * eval: (c-set-style "gnu")
 * End:
 */
