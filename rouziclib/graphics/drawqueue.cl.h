"enum\n{\n DQ_END = 0,\n DQ_ENTRY_START = 1,\n DQ_END_HEADER_SL,\n};\nenum dq_type\n{\n DQT_NOTYPE,\n DQT_BRACKET_OPEN,\n DQT_BRACKET_CLOSE,\n DQT_LINE_THIN_ADD,\n DQT_POINT_ADD,\n DQT_RECT_FULL,\n DQT_RECT_BLACK,\n DQT_PLAIN_FILL,\n DQT_GAIN,\n DQT_GAIN_PARAB,\n DQT_LUMA_COMPRESS,\n DQT_CIRCLE_FULL,\n DQT_CIRCLE_HOLLOW,\n DQT_BLIT_BILINEAR,\n DQT_BLIT_FLATTOP,\n DQT_BLIT_PHOTO,\n DQT_TEST1,\n};\nenum dq_blend\n{\n DQB_ADD,\n DQB_SUB,\n DQB_MUL,\n DQB_DIV,\n DQB_BLEND,\n DQB_SOLID,\n};\nfloat gaussian(float x)\n{\n return native_exp(-x*x);\n}\nfloat erfr(float x)\n{\n return 0.5f + 0.5f * erf(x);\n}\nfloat4 blend_pixel(float4 bg, float4 fg, int blendmode)\n{\n float4 pv;\n switch (blendmode)\n {\n  case DQB_ADD:\n   pv = bg + fg;\n   break;\n  case DQB_SUB:\n   pv = bg - fg;\n   break;\n  case DQB_MUL:\n   pv = bg * fg;\n   break;\n  case DQB_DIV:\n   pv = bg / fg;\n   break;\n  case DQB_BLEND:\n   pv = fg * fg.w + bg * (1.f - fg.w);\n   break;\n  case DQB_SOLID:\n   pv = fg;\n   break;\n }\n return pv;\n}\nfloat4 draw_line_thin_add(global float *le, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n const float gl = 4.f;\n float v, iradius;\n float2 p1, p2, d12, d12t1p, p4;\n float4 col;\n float line_len, d12s, u, d1p, d2p, d3p;\n p1.x = le[0];\n p1.y = le[1];\n p2.x = le[2];\n p2.y = le[3];\n iradius = le[4];\n col.s0 = le[5];\n col.s1 = le[6];\n col.s2 = le[7];\n col.s3 = le[8];\n d12 = p2-p1;\n d12s = d12.x*d12.x + d12.y*d12.y;\n if (d12s==0.f)\n  return pv;\n line_len = native_sqrt(d12s);\n d12t1p = (pf-p1) * d12;\n u = (d12t1p.x + d12t1p.y) / d12s;\n p4 = p1 + u * d12;\n d1p = u * line_len;\n d2p = d1p - line_len;\n d3p = fast_distance(p4, pf);\n d1p *= iradius;\n d2p *= iradius;\n d3p *= iradius;\n if (d1p <= -gl)\n  return pv;\n if (d2p >= gl)\n  return pv;\n v = erfr(d1p) - erfr(d2p);\n v *= gaussian(d3p);\n pv += v * col;\n return pv;\n}\nfloat4 draw_point_add(global float *le, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n const float gl = 4.f;\n float rad, d;\n float2 dp;\n float4 col;\n dp.x = le[0];\n dp.y = le[1];\n rad = le[2];\n col.s0 = le[3];\n col.s1 = le[4];\n col.s2 = le[5];\n col.s3 = 1.;\n d = fast_distance(dp, pf) * rad;\n if (d < gl)\n  pv += gaussian(d) * col;\n return pv;\n}\nfloat4 draw_rect_full_add(global float *le, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n float rad, d;\n float2 p0, p1, d0, d1, gv;\n float4 col;\n p0.x = le[0];\n p0.y = le[1];\n p1.x = le[2];\n p1.y = le[3];\n rad = le[4];\n col.s0 = le[5];\n col.s1 = le[6];\n col.s2 = le[7];\n col.s3 = 1.;\n d0 = (pf - p0) * rad;\n d1 = (pf - p1) * rad;\n gv.x = erfr(d0.x) - erfr(d1.x);\n gv.y = erfr(d0.y) - erfr(d1.y);\n pv += gv.x*gv.y * col;\n return pv;\n}\nfloat4 draw_black_rect(global float *le, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n float rad, d;\n float2 p0, p1, d0, d1, gv;\n p0.x = le[0];\n p0.y = le[1];\n p1.x = le[2];\n p1.y = le[3];\n rad = le[4];\n d0 = (pf - p0) * rad;\n d1 = (pf - p1) * rad;\n gv.x = erfr(d0.x) - erfr(d1.x);\n gv.y = erfr(d0.y) - erfr(d1.y);\n pv *= 1.f - gv.x*gv.y;\n return pv;\n}\nfloat4 draw_circle_full_add(global float *le, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n float4 col;\n float2 pc;\n float circrad, rad, dc, dn, df;\n pc.x = le[0];\n pc.y = le[1];\n circrad = le[2];\n rad = le[3];\n col.s0 = le[4];\n col.s1 = le[5];\n col.s2 = le[6];\n col.s3 = 1.;\n dc = fast_distance(pf, pc);\n dn = (circrad - dc) * rad;\n df = -(circrad + dc) * rad;\n pv += (erfr(dn) - erfr(df)) * col;\n return pv;\n}\nfloat4 draw_circle_hollow_add(global float *le, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n float4 col;\n float2 pc;\n float circrad, rad, dc, dn, df;\n pc.x = le[0];\n pc.y = le[1];\n circrad = le[2];\n rad = le[3];\n col.s0 = le[4];\n col.s1 = le[5];\n col.s2 = le[6];\n col.s3 = 1.;\n dc = fast_distance(pf, pc);\n dn = (circrad - dc) * rad;\n df = -(circrad + dc) * rad;\n pv += (gaussian(dn) + gaussian(df)) * col;\n return pv;\n}\nint pow_mod(int base, uint expon, uint mod)\n{\n int x = 1, power = base % mod;\n for (; expon > 0; expon >>= 1)\n {\n  if (expon & 1)\n   x = (x * power) % mod;\n  power = (power * power) % mod;\n }\n return x;\n}\nuint rand_minstd(uint pos)\n{\n return pow_mod(16807, pos, 2147483647);\n}\nuint rand_minstd16(uint pos)\n{\n return rand_minstd(pos) >> 13 & 0xFFFF;\n}\nuint rand_minstd32(uint pos)\n{\n return rand_minstd16(pos) << 16 | rand_minstd16(pos + 0x80000000);\n}\nfloat rand_minstd_01(uint pos)\n{\n return (float) rand_minstd32(pos) * 2.3283064370807973754314699618685e-10f;\n}\nfloat rand_minstd_exc01(uint pos)\n{\n return (float) (rand_minstd32(pos) + 1) * 0.00000000023283064365386962890625f;\n}\nfloat gaussian_rand_minstd(uint pos)\n{\n float r;\n pos <<= 1;\n r = native_sqrt(-2.f * native_log(rand_minstd_exc01(pos)));\n return r * native_sin(2.f*M_PI_F * rand_minstd_01(pos+1)) * M_SQRT1_2_F;\n}\nfloat gaussian_rand_minstd_approx(uint pos)\n{\n float r = ((float) rand_minstd(pos) - 1073741823.f) * 9.313225466e-10f;\n return copysign(0.8862269254f * native_sqrt(- native_log(1.f - r*r)), r);\n}\nfloat4 colour_blowout(float4 c)\n{\n float maxv, t, L;\n maxv = max(max(c.s0, c.s1), c.s2);\n if (maxv > 1.f)\n {\n  L = 0.16f*c.s0 + 0.73f*c.s1 + 0.11f*c.s2;\n  if (L < 1.f)\n  {\n   t = (1.f-L) / (maxv-L);\n   c.s0 = c.s0*t + L*(1.f-t);\n   c.s1 = c.s1*t + L*(1.f-t);\n   c.s2 = c.s2*t + L*(1.f-t);\n  }\n  else\n  {\n   c.s0 = c.s1 = c.s2 = 1.f;\n  }\n }\n return c;\n}\nfloat lsrgb(float linear)\n{\n if (linear <= 0.0031308f)\n  return linear * 12.92f;\n else\n  return 1.055f * pow(linear, 1.f/2.4f) - 0.055f;\n}\nfloat slrgb(float s)\n{\n if (s <= 0.04045f)\n  return s * (1.f/12.92f);\n else\n  return pow((s + 0.055f) * (1.f/1.055f), 2.4f);\n}\nfloat s8lrgb(float s8)\n{\n return slrgb(s8 * (1.f/255.f));\n}\nfloat apply_dithering(float pv, float dv)\n{\n const float threshold = 1.2f / 255.f;\n const float it = 1.f / threshold;\n const float rounding_offset = 0.5f / 255.f;\n if (pv < threshold)\n  if (pv <= 0.f)\n   return 0.f;\n  else\n   dv *= pv * it;\n if (pv > 1.f - threshold)\n  if (pv >= 1.f)\n   return 1.f;\n  else\n   dv *= (1.f-pv) * it;\n return pv += dv + rounding_offset;\n}\nfloat4 linear_to_srgb(float4 pl0, uint seed)\n{\n float4 pl1;\n float dith;\n uchar4 ps;\n const float dith_scale = M_SQRT1_2_F / 255.f;\n pl0 = mix(colour_blowout(pl0), clamp(pl0, 0.f, 1.f), 0.666f);\n pl1.s0 = lsrgb(pl0.s0);\n pl1.s1 = lsrgb(pl0.s1);\n pl1.s2 = lsrgb(pl0.s2);\n dith = gaussian_rand_minstd_approx(seed) * dith_scale;\n pl1.s0 = apply_dithering(pl1.s0, dith);\n pl1.s1 = apply_dithering(pl1.s1, dith);\n pl1.s2 = apply_dithering(pl1.s2, dith);\n if (255.f < 255.f)\n  pl1 = round(pl1 * 255.f) / 255.f;\n return pl1;\n}\nfloat4 read_sqrgb_pixel(global uint *im, int index)\n{\n float4 pv;\n uint4 pvi;\n uint v;\n const float mul_rb = 1.f / (1023.f*1023.f);\n const float mul_g = 1.f / (4092.f*4092.f);\n const float4 mul_pvi = (float4) (mul_rb, mul_g, mul_rb, 1.f);\n v = im[index];\n pvi.z = v >> 22;\n pvi.y = (v >> 10) & 4095;\n pvi.x = v & 1023;\n pvi.w = 1;\n pv = convert_float4(pvi*pvi) * mul_pvi;\n return pv;\n}\nfloat4 read_srgb_pixel(global uint *im, int index)\n{\n float4 pv;\n uint v;\n v = im[index];\n pv.z = s8lrgb((v >> 16) & 255);\n pv.y = s8lrgb((v >> 8) & 255);\n pv.x = s8lrgb(v & 255);\n pv.w = 1.;\n return pv;\n}\nfloat4 raw_yuv_to_lrgb(float3 raw, float depth_mul)\n{\n float y, u, v, r, g, b;\n float4 pv;\n raw *= depth_mul;\n y = raw.x - 16.f;\n y *= 255.f / 219.f;\n u = raw.y - 128.f;\n v = raw.z - 128.f;\n r = y + 1.596f * v;\n        g = y - 0.813f * v - 0.391f * u;\n        b = y + 2.018f * u;\n pv.x = s8lrgb(r);\n pv.y = s8lrgb(g);\n pv.z = s8lrgb(b);\n pv.w = 1.;\n return pv;\n}\nfloat4 read_yuv420p8_pixel(global uchar *im, int2 im_dim, int2 i)\n{\n float4 pv;\n float y, u, v, r, g, b;\n int2 im_dimh = im_dim / 2;\n int size_full = im_dim.x*im_dim.y, size_half = size_full/4, y_index, uv_index;\n global uchar *u_plane, *v_plane;\n u_plane = &im[size_full];\n v_plane = &im[size_full + size_half];\n y_index = i.y * im_dim.x + i.x;\n uv_index = i.y/2 * im_dimh.x + i.x/2;\n pv = raw_yuv_to_lrgb( (float3) (im[y_index], u_plane[uv_index], v_plane[uv_index]), 1.f );\n return pv;\n}\nfloat4 read_yuv420pN_pixel(global ushort *im, int2 im_dim, int2 i, float depth_mul)\n{\n float4 pv;\n float y, u, v, r, g, b;\n int2 im_dimh = im_dim / 2;\n int size_full = im_dim.x*im_dim.y, size_half = size_full/4, y_index, uv_index;\n global ushort *u_plane, *v_plane;\n u_plane = &im[size_full];\n v_plane = &im[size_full + size_half];\n y_index = i.y * im_dim.x + i.x;\n uv_index = i.y/2 * im_dimh.x + i.x/2;\n pv = raw_yuv_to_lrgb( (float3) (im[y_index], u_plane[uv_index], v_plane[uv_index]), depth_mul );\n return pv;\n}\nfloat calc_flattop_weight(float2 pif, float2 i, float2 knee, float2 slope, float2 pscale)\n{\n float2 d, w;\n d = fabs(pif - i);\n d = max(d, knee);\n w = slope * (d - pscale);\n return w.x * w.y;\n}\nfloat4 image_filter_flattop(global float4 *im, int2 im_dim, const int fmt, float2 pif, float2 pscale, float2 slope)\n{\n float4 pv = 0.f;\n float2 knee, i, start, end;\n float depth_mul=0.25f;\n knee = 0.5f - fabs(fmod(pscale, 1.f) - 0.5f);\n start = max(0.f, ceil(pif - pscale));\n end = min(convert_float2(im_dim - 1), floor(pif + pscale));\n if (fmt==0)\n  for (i.y = start.y; i.y <= end.y; i.y+=1.f)\n   for (i.x = start.x; i.x <= end.x; i.x+=1.f)\n    pv += im[(int) i.y * im_dim.x + (int) i.x] * calc_flattop_weight(pif, i, knee, slope, pscale);\n else if (fmt==1)\n  for (i.y = start.y; i.y <= end.y; i.y+=1.f)\n   for (i.x = start.x; i.x <= end.x; i.x+=1.f)\n    pv += read_sqrgb_pixel(im, (int) i.y * im_dim.x + (int) i.x) * calc_flattop_weight(pif, i, knee, slope, pscale);\n else if (fmt==2)\n  for (i.y = start.y; i.y <= end.y; i.y+=1.f)\n   for (i.x = start.x; i.x <= end.x; i.x+=1.f)\n    pv += read_srgb_pixel(im, (int) i.y * im_dim.x + (int) i.x) * calc_flattop_weight(pif, i, knee, slope, pscale);\n else if (fmt==10)\n  for (i.y = start.y; i.y <= end.y; i.y+=1.f)\n   for (i.x = start.x; i.x <= end.x; i.x+=1.f)\n    pv += read_yuv420p8_pixel(im, im_dim, convert_int2(i)) * calc_flattop_weight(pif, i, knee, slope, pscale);\n else if (fmt==11 || fmt==12)\n {\n  switch (fmt)\n  {\n   case 11: depth_mul = 0.25f; break;\n   case 12: depth_mul = 0.0625f; break;\n  }\n  for (i.y = start.y; i.y <= end.y; i.y+=1.f)\n   for (i.x = start.x; i.x <= end.x; i.x+=1.f)\n    pv += read_yuv420pN_pixel(im, im_dim, convert_int2(i), depth_mul) * calc_flattop_weight(pif, i, knee, slope, pscale);\n }\n return pv;\n}\nfloat4 blit_sprite_flattop(global uint *lei, global uchar *data_cl, float4 pv)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const float2 pf = convert_float2(p);\n global float *lef = lei;\n global float4 *im;\n int2 im_dim;\n int fmt;\n float2 pscale, pos, pif, slope;\n im = (global float4 *) &data_cl[lei[0]+(lei[1]<<32)];\n im_dim.x = lei[2];\n im_dim.y = lei[3];\n pscale.x = lef[4];\n pscale.y = lef[5];\n pos.x = lef[6];\n pos.y = lef[7];\n fmt = lei[8];\n slope.x = lef[9];\n slope.y = lef[10];\n pif = pscale * (pf + pos);\n pscale = max(1.f, pscale);\n pv += image_filter_flattop(im, im_dim, fmt, pif, pscale, slope);\n return pv;\n}\nfloat Lab_L_to_linear(float t)\n{\n const float stn=6.f/29.f;\n t = (t+0.16f) / 1.16f;\n if (t > stn)\n  return t*t*t;\n else\n  return 3.f*stn*stn*(t - 4.f/29.f);\n}\nfloat linear_to_Lab_L(float t)\n{\n const float thr = 6.f/29.f, thr3 = thr*thr*thr;\n if (t > thr3)\n  t = cbrt(t);\n else\n  t = t * 841.f/108.f + 4.f/29.f;\n return 1.16f * t - 0.16f;\n}\nfloat4 gain_parabolic(float4 pv, float gain)\n{\n pv = min(pv, 1.f);\n pv = 1.f - pow((1.f-pv), gain);\n return pv;\n}\nfloat4 luma_compression(float4 pv0, float lvl_lin)\n{\n float4 pv1;\n float grey0_lin, grey0_perc, grey1_perc, grey1_lin, white1_perc, white1_lin, lvl_perc, ratio;\n grey0_lin = 0.16f*pv0.s0 + 0.73f*pv0.s1 + 0.11f*pv0.s2;\n if (grey0_lin==0.f)\n  return pv0;\n grey0_perc = linear_to_Lab_L(grey0_lin);\n lvl_perc = linear_to_Lab_L(lvl_lin);\n grey1_perc = grey0_perc + lvl_perc;\n grey1_lin = Lab_L_to_linear(grey1_perc) - lvl_lin;\n white1_perc = 1.f + lvl_perc;\n white1_lin = Lab_L_to_linear(white1_perc) - lvl_lin;\n ratio = (grey1_lin / grey0_lin) / white1_lin;\n pv1 = pv0 * ratio;\n return pv1;\n}\nfloat4 drawgradienttest(float4 pv)\n{\n const float2 pf = (float2) (get_global_id(0), get_global_id(1));\n const float2 ss = (float2) (get_global_size(0), get_global_size(1));\n const float2 c = ss * 0.5f;\n pv = -(pf.x - c.x) / (ss.x * 0.1f);\n return pv;\n}\nfloat4 draw_plain_fill_add(global float *le, float4 pv)\n{\n float4 col;\n col.s0 = le[0];\n col.s1 = le[1];\n col.s2 = le[2];\n col.s3 = 1.;\n pv += col;\n return pv;\n}\nfloat4 draw_queue(global float *df, global int *poslist, global int *entrylist, global uchar *data_cl, const int sector_w, const int sector_size)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const int sec = (p.y >> sector_size) * sector_w + (p.x >> sector_size);\n global int *di = (global int *) df;\n int i, eli, entry_count, qi;\n float4 pv = 0.f;\n int brlvl = 0;\n float4 br[4];\n eli = poslist[sec];\n if (eli < 0)\n  return pv;\n entry_count = entrylist[eli];\n for (i=0; i<entry_count; i++)\n {\n  qi = entrylist[eli + i + 1];\n  switch (di[qi])\n  {\n   case DQT_BRACKET_OPEN:\n    br[brlvl] = pv;\n    pv = 0.f;\n    brlvl++;\n    break;\n   case DQT_BRACKET_CLOSE:\n    brlvl--;\n    pv = blend_pixel(br[brlvl], pv, di[qi+1]);\n    break;\n   case DQT_LINE_THIN_ADD: pv = draw_line_thin_add(&df[qi+1], pv); break;\n   case DQT_POINT_ADD: pv = draw_point_add(&df[qi+1], pv); break;\n   case DQT_RECT_FULL: pv = draw_rect_full_add(&df[qi+1], pv); break;\n   case DQT_RECT_BLACK: pv = draw_black_rect(&df[qi+1], pv); break;\n   case DQT_PLAIN_FILL: pv = draw_plain_fill_add(&df[qi+1], pv); break;\n   case DQT_GAIN: pv = pv * df[qi+1]; break;\n   case DQT_GAIN_PARAB: pv = gain_parabolic(pv, df[qi+1]); break;\n   case DQT_LUMA_COMPRESS: pv = luma_compression(pv, df[qi+1]); break;\n   case DQT_CIRCLE_FULL: pv = draw_circle_full_add(&df[qi+1], pv); break;\n   case DQT_CIRCLE_HOLLOW: pv = draw_circle_hollow_add(&df[qi+1], pv); break;\n   case DQT_BLIT_FLATTOP: pv = blit_sprite_flattop(&df[qi+1], data_cl, pv); break;\n   case DQT_TEST1: pv = drawgradienttest(pv); break;\n   default:\n    break;\n  }\n }\n return pv;\n}\nkernel void draw_queue_srgb_kernel(const ulong df_index, const ulong poslist_index, const ulong entrylist_index, global uchar *data_cl, write_only image2d_t srgb, const int sector_w, const int sector_size, const int randseed)\n{\n const int2 p = (int2) (get_global_id(0), get_global_id(1));\n const int fbi = p.y * get_global_size(0) + p.x;\n float4 pv;\n global float *df = &data_cl[df_index];\n global int *poslist = &data_cl[poslist_index];\n global int *entrylist = &data_cl[entrylist_index];\n pv = draw_queue(df, poslist, entrylist, data_cl, sector_w, sector_size);\n if (pv.s0==0.f)\n if (pv.s1==0.f)\n if (pv.s2==0.f)\n  return ;\n write_imagef(srgb, p, linear_to_srgb(pv, randseed+fbi));\n}\n";