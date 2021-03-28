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
#include <benker_plugin/benker_plugin.h>
#include <vnet/classify/vnet_classify.h>

typedef struct 
{
  u8 routine; // currently only got 3 routines, 1 or 2, and 0 for default
  u32 next_index;
  u32 routine_tx_sw_if_index;
} benker_plugin_trace_t;

#ifndef CLIB_MARCH_VARIANT

/* packet trace format function */
static u8 * format_benker_plugin_trace (u8 * s, va_list * args)
{
  CLIB_UNUSED (vlib_main_t * vm) = va_arg (*args, vlib_main_t *);
  CLIB_UNUSED (vlib_node_t * node) = va_arg (*args, vlib_node_t *);
  benker_plugin_trace_t * t = va_arg (*args, benker_plugin_trace_t *);
  
  s = format (s, "BENKER_PLUGIN: routine %d, next index %d, routine_tx_sw_if_index: %d\n",
              t->routine, t->next_index, t->routine_tx_sw_if_index);
  return s;
}

vlib_node_registration_t benker_plugin_node;

#endif /* CLIB_MARCH_VARIANT */

#define foreach_benker_plugin_error \
_(HANDLED, "Benker plugin handled packets")

typedef enum {
#define _(sym,str) BENKER_PLUGIN_ERROR_##sym,
  foreach_benker_plugin_error
#undef _
  BENKER_PLUGIN_N_ERROR,
} benker_plugin_error_t;

#ifndef CLIB_MARCH_VARIANT
static char * benker_plugin_error_strings[] = 
{
#define _(sym,string) string,
  foreach_benker_plugin_error
#undef _
};
#endif /* CLIB_MARCH_VARIANT */

typedef enum 
{
  BENKER_PLUGIN_NEXT_INTERFACE_OUTPUT,
  BENKER_PLUGIN_NEXT_ERROR_DROP,
  BENKER_PLUGIN_N_NEXT,
} benker_plugin_next_t;


VLIB_NODE_FN (benker_plugin_node) (vlib_main_t * vm,
		  vlib_node_runtime_t * node,
		  vlib_frame_t * frame)
{
  benker_plugin_main_t * bmp = &benker_plugin_main;
  u32 n_left_from, * from, * to_next;
  benker_plugin_next_t next_index;
  u32 pkts_handled = 0;

  from = vlib_frame_vector_args (frame);
  n_left_from = frame->n_vectors;
  next_index = node->cached_next_index;

  while (n_left_from > 0)
    {
      u32 n_left_to_next;

      vlib_get_next_frame (vm, node, next_index,
			   to_next, n_left_to_next);

      while (n_left_from >= 4 && n_left_to_next >= 2 && false /* focus on 1 pkt handling first */)
	{
          u32 next0 = BENKER_PLUGIN_NEXT_INTERFACE_OUTPUT;
          u32 next1 = BENKER_PLUGIN_NEXT_INTERFACE_OUTPUT;
          u32 sw_if_index0, sw_if_index1;
          u32 bi0, bi1;
	  vlib_buffer_t * b0, * b1;
          
	  /* Prefetch next iteration. */
	  {
	    vlib_buffer_t * p2, * p3;
            
	    p2 = vlib_get_buffer (vm, from[2]);
	    p3 = vlib_get_buffer (vm, from[3]);
            
	    vlib_prefetch_buffer_header (p2, LOAD);
	    vlib_prefetch_buffer_header (p3, LOAD);

	    CLIB_PREFETCH (p2->data, CLIB_CACHE_LINE_BYTES, STORE);
	    CLIB_PREFETCH (p3->data, CLIB_CACHE_LINE_BYTES, STORE);
	  }

          /* speculatively enqueue b0 and b1 to the current next frame */
	  to_next[0] = bi0 = from[0];
	  to_next[1] = bi1 = from[1];
	  from += 2;
	  to_next += 2;
	  n_left_from -= 2;
	  n_left_to_next -= 2;

	  b0 = vlib_get_buffer (vm, bi0);
	  b1 = vlib_get_buffer (vm, bi1);

          ASSERT (b0->current_data == 0);
          ASSERT (b1->current_data == 0);

          sw_if_index0 = vnet_buffer(b0)->sw_if_index[VLIB_RX];
          sw_if_index1 = vnet_buffer(b1)->sw_if_index[VLIB_RX];

          /* Send pkt back out the RX interface */
          vnet_buffer(b0)->sw_if_index[VLIB_TX] = sw_if_index0;
          vnet_buffer(b1)->sw_if_index[VLIB_TX] = sw_if_index1;

          pkts_handled += 2;

          if (PREDICT_FALSE((node->flags & VLIB_NODE_FLAG_TRACE)))
            {
              if (b0->flags & VLIB_BUFFER_IS_TRACED) 
                {

                  }
                if (b1->flags & VLIB_BUFFER_IS_TRACED) 
                  {

                  }
              }
            
            /* verify speculative enqueues, maybe switch current next frame */
            vlib_validate_buffer_enqueue_x2 (vm, node, next_index,
                                             to_next, n_left_to_next,
                                             bi0, bi1, next0, next1);
        }

      while (n_left_from > 0 && n_left_to_next > 0)
	{
          u32 bi0;
	  vlib_buffer_t * b0;
          u32 next0 = BENKER_PLUGIN_NEXT_INTERFACE_OUTPUT;
          u32 sw_if_index0;

          /* speculatively enqueue b0 to the current next frame */
	  bi0 = from[0];
	  to_next[0] = bi0;
	  from += 1;
	  to_next += 1;
	  n_left_from -= 1;
	  n_left_to_next -= 1;

	  b0 = vlib_get_buffer (vm, bi0);
          /* 
           * Direct from the driver, we should be at offset 0
           * aka at &b0->data[0]
           */
          ASSERT (b0->current_data == 0);
          
          sw_if_index0 = vnet_buffer(b0)->sw_if_index[VLIB_RX];

          u8 routine = 0;
          uword *p0 = hash_get (bmp->output_infc_map, sw_if_index0);
          u32 routine1_sw_if_index;
          u32 routine2_sw_if_index;
          /* get the mapping */
          if (PREDICT_FALSE (p0 == NULL))
            {
              clib_warning("benker_plugin sw_inf_index: %d, value not found", sw_if_index0);
              next0 = BENKER_PLUGIN_NEXT_ERROR_DROP;
            }
          else
            {
              clib_warning ("benker_plugin routine1: %d, routine2: %d", routine1_sw_if_index, routine2_sw_if_index);
              routine1_sw_if_index = p0[0] >> 32;
              routine2_sw_if_index = p0[0];
              sw_if_index0 = routine1_sw_if_index;
            }

          /* Send pkt back out the RX interface */
          vnet_buffer(b0)->sw_if_index[VLIB_TX] = sw_if_index0;

          if (PREDICT_FALSE((node->flags & VLIB_NODE_FLAG_TRACE) 
                            && (b0->flags & VLIB_BUFFER_IS_TRACED))) {
            benker_plugin_trace_t *t = 
               vlib_add_trace (vm, node, b0, sizeof (*t));
            t->next_index = next0;
            t->routine = routine;
            t->routine_tx_sw_if_index = ~0;
            if (routine == 1) 
              {
                t->routine_tx_sw_if_index = routine1_sw_if_index;
              }
            else if (routine == 2)
              {
                t->routine_tx_sw_if_index = routine2_sw_if_index;
              }
            }
            
          pkts_handled += 1;

          /* verify speculative enqueue, maybe switch current next frame */
	  vlib_validate_buffer_enqueue_x1 (vm, node, next_index,
					   to_next, n_left_to_next,
					   bi0, next0);
	}

      vlib_put_next_frame (vm, node, next_index, n_left_to_next);
    }

  vlib_node_increment_counter (vm, benker_plugin_node.index, 
                               BENKER_PLUGIN_ERROR_HANDLED, pkts_handled);
  return frame->n_vectors;
}

/* *INDENT-OFF* */
#ifndef CLIB_MARCH_VARIANT
VLIB_REGISTER_NODE (benker_plugin_node) = 
{
  .name = "benker_plugin",
  .vector_size = sizeof (u32),
  .format_trace = format_benker_plugin_trace,
  .type = VLIB_NODE_TYPE_INTERNAL,
  
  .n_errors = ARRAY_LEN(benker_plugin_error_strings),
  .error_strings = benker_plugin_error_strings,

  .n_next_nodes = BENKER_PLUGIN_N_NEXT,

  /* edit / add dispositions here */
  .next_nodes = {
        [BENKER_PLUGIN_NEXT_INTERFACE_OUTPUT] = "interface-output",
        [BENKER_PLUGIN_NEXT_ERROR_DROP] = "error-drop",
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
