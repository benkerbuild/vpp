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

#include <vnet/policer/policer.h>
#include <vnet/policer/police_inlines.h>

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
_(HANDLED, "Benker plugin handled packets") \
_(ROUTINE0, "Benker plugin default-routine packets") \
_(ROUTINE1, "Benker plugin routine-1 packets") \
_(ROUTINE2, "Benker plugin routine-2 packets") \
_(POLICER_DROP, "Benker plugin policer drop packets (routine-1)")

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
  BENKER_PLUGIN_NEXT_ERROR_DROP,
  BENKER_PLUGIN_NEXT_INTERFACE_OUTPUT,
  BENKER_PLUGIN_NEXT_BENKER_STATS,
  BENKER_PLUGIN_N_NEXT,
} benker_plugin_next_t;


VLIB_NODE_FN (benker_plugin_node) (vlib_main_t * vm,
		  vlib_node_runtime_t * node,
		  vlib_frame_t * frame)
{
  benker_plugin_main_t * bmp = &benker_plugin_main;
  vnet_classify_main_t *vcm = bmp->pcm->vnet_classify_main;
  f64 now = vlib_time_now (vm);
  u32 n_left_from, * from, * to_next;
  benker_plugin_next_t next_index;
  u32 pkts_handled = 0;
  u32 pkts_routines[3] = {0, 0, 0};
  uword *output_intfc_value = NULL;    // first 32-bit is output interface for routine0, 1 ;; second 32-bit for routine2
  u32 last_pkt_sw_if_index = ~0;     // for reducing the hash-lookup if same sw interface
  u64 time_in_policer_periods;

  time_in_policer_periods =
    clib_cpu_time_now () >> POLICER_TICKS_PER_PERIOD_SHIFT;

  from = vlib_frame_vector_args (frame);
  n_left_from = frame->n_vectors;
  next_index = node->cached_next_index;

  /* compute packet hash & assign hash table index to packets */
  while (n_left_from > 0)
    {
      vlib_buffer_t *b0;
      u32 bi0;
      u8 *h0;
      u32 sw_if_index0;
      u32 table_index0;
      vnet_classify_table_t *t0;

      bi0 = from[0];
      b0 = vlib_get_buffer (vm, bi0);
      h0 = b0->data;

      sw_if_index0 = vnet_buffer (b0)->sw_if_index[VLIB_RX];
      // check if the classify table exist for that intfc
      if (PREDICT_FALSE(vec_len(bmp->pcm->classify_table_index_by_sw_if_index[POLICER_CLASSIFY_TABLE_L2]) < sw_if_index0+1))
        {
          vnet_buffer (b0)->l2_classify.table_index = ~0;
          from++;
          n_left_from--;
          continue;
        }
      else
        table_index0 = 
      bmp->pcm->classify_table_index_by_sw_if_index[POLICER_CLASSIFY_TABLE_L2][sw_if_index0];
      
      t0 = pool_elt_at_index (vcm->tables, table_index0);
      vnet_buffer (b0)->l2_classify.hash = 
    vnet_classify_hash_packet (t0, (u8 *) h0);

      vnet_buffer (b0)->l2_classify.table_index = table_index0;
      vnet_classify_prefetch_bucket (t0, vnet_buffer (b0)->l2_classify.hash);

      from++;
      n_left_from--;
    }

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
          u32 routines_sw_if_index0[3] = {~0, ~0, ~0};  // routine0 same as routine1
          u8 routine0 = 0;  // which routine it goes for the packet
          u32 table_index0;
          vnet_classify_table_t *t0;
          vnet_classify_entry_t *e0;
          u64 hash0;
          u8 *h0;
          u8 act0;

          /* speculatively enqueue b0 to the current next frame */
	  bi0 = from[0];
	  to_next[0] = bi0;
	  from += 1;
	  to_next += 1;
	  n_left_from -= 1;
	  n_left_to_next -= 1;

	  b0 = vlib_get_buffer (vm, bi0);
    h0 = b0->data;
    table_index0 = vnet_buffer (b0)->l2_classify.table_index;
    e0 = 0;
    t0 = 0;
          /* 
           * Direct from the driver, we should be at offset 0
           * aka at &b0->data[0]
           */
          ASSERT (b0->current_data == 0);
          
          /* get the mapping */
          if (PREDICT_FALSE (last_pkt_sw_if_index != vnet_buffer(b0)->sw_if_index[VLIB_RX]))
            {
              last_pkt_sw_if_index = vnet_buffer(b0)->sw_if_index[VLIB_RX];
              output_intfc_value = hash_get (bmp->output_infc_map, last_pkt_sw_if_index);
            }

          ASSERT (output_intfc_value != NULL);
          
          routines_sw_if_index0[0] = routines_sw_if_index0[1] = output_intfc_value[0] >> 32; // routine0, 1, first 32-bit
          routines_sw_if_index0[2] = output_intfc_value[0];       // routine 2, second 32-bit

          /*
              main logic, pseudocode for the idea (in python sytle)

              entry = get_entry(hash_table0, pkt)
              if has_entry(entry): # routine-1
                action = policer(entry)
                if action == DROP:
                  put_pkt_to_drop(pkt)
                else:
                  update_gtpu_stats(pkt)
                  put_pkt_to_routine1_intfc(pkt)
              else:
                entry = get_entry(hash_table1, pkt)
                if has_entry(entry):  # routine-2
                  put_pkt_to_routine2_intfc(pkt)
                else:
                  put_pkt_to_routine1_intfc(pkt)  # default routine same output intfc as routine-1
              
          */
          routine0 = 0;   // default routine
          if (PREDICT_TRUE (table_index0 != ~0))
            {
              hash0 = vnet_buffer (b0)->l2_classify.hash;
              t0 = pool_elt_at_index (vcm->tables, table_index0);
              e0 = vnet_classify_find_entry (t0, (u8 *) h0, hash0, now);

              if (e0)
                {
                  routine0 = 1;
                  act0 = vnet_policer_police (vm,
                                  b0,
                                  e0->next_index,
                                  time_in_policer_periods,
                                  e0->opaque_index);
                  if (PREDICT_FALSE (act0 == SSE2_QOS_ACTION_DROP))
                    {
                      next0 = BENKER_PLUGIN_NEXT_ERROR_DROP;
                      b0->error = node->errors[BENKER_PLUGIN_ERROR_POLICER_DROP];
                    }
                  else
                    {
                      next0 = BENKER_PLUGIN_NEXT_BENKER_STATS;
                    }
                }
              else if (PREDICT_TRUE (t0->next_table_index != ~0))
                {
                  t0 = pool_elt_at_index (vcm->tables, t0->next_table_index);
                  e0 = vnet_classify_find_entry (t0, (u8 *) h0, hash0, now);
                  if (e0)
                    {
                      routine0 = 2;
                    }
                }
            }
          vnet_buffer (b0)->sw_if_index[VLIB_TX] = routines_sw_if_index0[routine0];
          

          if (PREDICT_FALSE((node->flags & VLIB_NODE_FLAG_TRACE) 
                            && (b0->flags & VLIB_BUFFER_IS_TRACED))) {
            benker_plugin_trace_t *t0 = 
               vlib_add_trace (vm, node, b0, sizeof (*t0));
            t0->next_index = next0;
            t0->routine = routine0;
            t0->routine_tx_sw_if_index = routines_sw_if_index0[routine0];
            }
            
          pkts_handled += 1;
          pkts_routines[routine0] += 1;

          /* verify speculative enqueue, maybe switch current next frame */
	  vlib_validate_buffer_enqueue_x1 (vm, node, next_index,
					   to_next, n_left_to_next,
					   bi0, next0);
	}

      vlib_put_next_frame (vm, node, next_index, n_left_to_next);
    }

  vlib_node_increment_counter (vm, benker_plugin_node.index, 
                               BENKER_PLUGIN_ERROR_HANDLED, pkts_handled);
  vlib_node_increment_counter (vm, benker_plugin_node.index, 
                               BENKER_PLUGIN_ERROR_ROUTINE0, pkts_routines[0]);
  vlib_node_increment_counter (vm, benker_plugin_node.index, 
                               BENKER_PLUGIN_ERROR_ROUTINE1, pkts_routines[1]);
  vlib_node_increment_counter (vm, benker_plugin_node.index, 
                               BENKER_PLUGIN_ERROR_ROUTINE2, pkts_routines[2]);
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
        [BENKER_PLUGIN_NEXT_ERROR_DROP] = "error-drop",
        [BENKER_PLUGIN_NEXT_INTERFACE_OUTPUT] = "interface-output",
        [BENKER_PLUGIN_NEXT_BENKER_STATS] = "benker_stats",
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
