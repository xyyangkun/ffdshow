/*****************************************************************************
 * cpu.h: h264 encoder library
 *****************************************************************************
 * Copyright (C) 2003 Laurent Aimar
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

#ifndef _CPU_H
#define _CPU_H 1

uint32_t x264_cpu_detect( void );
int      x264_cpu_num_processors( void );

/* probably MMX(EXT) centric but .... */
void     x264_cpu_restore( uint32_t cpu );

/* kluge:
 * gcc can't give variables any greater alignment than the stack frame has.
 * We need 16 byte alignment for SSE2, so here we make sure that the stack is
 * aligned to 16 bytes.
 * gcc 4.2 introduced __attribute__((force_align_arg_pointer)) to fix this
 * problem, but I don't want to require such a new version.
 * This applies only to x86_32, since other architectures that need alignment
 * also have ABIs that ensure aligned stack. */
#ifdef ARCH_X86
void x264_stack_align( void (*func)(x264_t*), x264_t *arg );
#else
#define x264_stack_align(func,arg) func(arg)
#endif

#endif
