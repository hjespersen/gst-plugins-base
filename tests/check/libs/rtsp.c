/* GStreamer unit tests for the RTSP support library
 *
 * Copyright (C) 2010 Andy Wingo <wingo@oblong.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/check/gstcheck.h>

#include <gst/rtsp/gstrtspurl.h>
#include <gst/rtsp/gstrtsprange.h>
#include <string.h>

GST_START_TEST (test_rtsp_url_basic)
{
  GstRTSPUrl *url = NULL;
  GstRTSPResult res;

  res = gst_rtsp_url_parse ("rtsp://localhost/foo/bar", &url);
  fail_unless (res == GST_RTSP_OK);
  fail_unless (url != NULL);
  fail_unless (url->transports & GST_RTSP_LOWER_TRANS_TCP);
  fail_unless (url->transports & GST_RTSP_LOWER_TRANS_UDP);
  fail_unless (url->transports & GST_RTSP_LOWER_TRANS_UDP_MCAST);
  fail_unless (url->family == GST_RTSP_FAM_INET);
  fail_unless (!url->user);
  fail_unless (!url->passwd);
  fail_unless (!strcmp (url->host, "localhost"));
  /* fail_unless (url->port == GST_RTSP_DEFAULT_PORT); */
  fail_unless (!strcmp (url->abspath, "/foo/bar"));
  fail_unless (!url->query);

  gst_rtsp_url_free (url);
}

GST_END_TEST;

GST_START_TEST (test_rtsp_url_components_1)
{
  GstRTSPUrl *url = NULL;
  GstRTSPResult res;
  gchar **comps = NULL;

  res = gst_rtsp_url_parse ("rtsp://localhost/foo/bar", &url);
  fail_unless (res == GST_RTSP_OK);
  fail_unless (url != NULL);

  comps = gst_rtsp_url_decode_path_components (url);
  fail_unless (comps != NULL);
  fail_unless (g_strv_length (comps) == 3);
  fail_unless (!strcmp (comps[0], ""));
  fail_unless (!strcmp (comps[1], "foo"));
  fail_unless (!strcmp (comps[2], "bar"));

  g_strfreev (comps);
  gst_rtsp_url_free (url);
}

GST_END_TEST;

GST_START_TEST (test_rtsp_url_components_2)
{
  GstRTSPUrl *url = NULL;
  GstRTSPResult res;
  gchar **comps = NULL;

  res = gst_rtsp_url_parse ("rtsp://localhost/foo%2Fbar/qux%20baz", &url);
  fail_unless (res == GST_RTSP_OK);
  fail_unless (url != NULL);

  comps = gst_rtsp_url_decode_path_components (url);
  fail_unless (comps != NULL);
  fail_unless (g_strv_length (comps) == 3);
  fail_unless (!strcmp (comps[0], ""));
  fail_unless (!strcmp (comps[1], "foo/bar"));
  fail_unless (!strcmp (comps[2], "qux baz"));

  g_strfreev (comps);
  gst_rtsp_url_free (url);
}

GST_END_TEST;

GST_START_TEST (test_rtsp_url_components_3)
{
  GstRTSPUrl *url = NULL;
  GstRTSPResult res;
  gchar **comps = NULL;

  res = gst_rtsp_url_parse ("rtsp://localhost/foo%00bar/qux%20baz", &url);
  fail_unless (res == GST_RTSP_OK);
  fail_unless (url != NULL);

  comps = gst_rtsp_url_decode_path_components (url);
  fail_unless (comps != NULL);
  fail_unless (g_strv_length (comps) == 3);
  fail_unless (!strcmp (comps[0], ""));
  fail_unless (!strcmp (comps[1], "foo%00bar"));
  fail_unless (!strcmp (comps[2], "qux baz"));

  g_strfreev (comps);
  gst_rtsp_url_free (url);
}

GST_END_TEST;

GST_START_TEST (test_rtsp_range_npt)
{
  GstRTSPTimeRange *range;

  fail_unless (gst_rtsp_range_parse ("npt=", &range) == GST_RTSP_EINVAL);
  fail_unless (gst_rtsp_range_parse ("npt=0", &range) == GST_RTSP_EINVAL);
  fail_unless (gst_rtsp_range_parse ("npt=-", &range) == GST_RTSP_EINVAL);
  fail_unless (gst_rtsp_range_parse ("npt=now", &range) == GST_RTSP_EINVAL);

  fail_unless (gst_rtsp_range_parse ("npt=-now", &range) == GST_RTSP_OK);
  fail_unless (range->unit == GST_RTSP_RANGE_NPT);
  fail_unless (range->min.type == GST_RTSP_TIME_END);
  fail_unless (range->max.type == GST_RTSP_TIME_NOW);
  gst_rtsp_range_free (range);

  fail_unless (gst_rtsp_range_parse ("npt=now-now", &range) == GST_RTSP_OK);
  fail_unless (range->unit == GST_RTSP_RANGE_NPT);
  fail_unless (range->min.type == GST_RTSP_TIME_NOW);
  fail_unless (range->max.type == GST_RTSP_TIME_NOW);
  gst_rtsp_range_free (range);

  fail_unless (gst_rtsp_range_parse ("npt=now-", &range) == GST_RTSP_OK);
  fail_unless (range->unit == GST_RTSP_RANGE_NPT);
  fail_unless (range->min.type == GST_RTSP_TIME_NOW);
  fail_unless (range->max.type == GST_RTSP_TIME_END);
  gst_rtsp_range_free (range);

  fail_unless (gst_rtsp_range_parse ("npt=now-34.12", &range) == GST_RTSP_OK);
  fail_unless (range->unit == GST_RTSP_RANGE_NPT);
  fail_unless (range->min.type == GST_RTSP_TIME_NOW);
  fail_unless (range->max.type == GST_RTSP_TIME_SECONDS);
  fail_unless (range->max.seconds == 34.12);
  gst_rtsp_range_free (range);

  fail_unless (gst_rtsp_range_parse ("npt=23,89-now", &range) == GST_RTSP_OK);
  fail_unless (range->unit == GST_RTSP_RANGE_NPT);
  fail_unless (range->min.type == GST_RTSP_TIME_SECONDS);
  fail_unless (range->min.seconds == 23.89);
  fail_unless (range->max.type == GST_RTSP_TIME_NOW);
  gst_rtsp_range_free (range);

  fail_unless (gst_rtsp_range_parse ("npt=-12.09", &range) == GST_RTSP_OK);
  fail_unless (range->unit == GST_RTSP_RANGE_NPT);
  fail_unless (range->min.type == GST_RTSP_TIME_END);
  fail_unless (range->max.type == GST_RTSP_TIME_SECONDS);
  fail_unless (range->max.seconds == 12.09);
  gst_rtsp_range_free (range);

  fail_unless (gst_rtsp_range_parse ("npt=0-", &range) == GST_RTSP_OK);
  fail_unless (range->unit == GST_RTSP_RANGE_NPT);
  fail_unless (range->min.type == GST_RTSP_TIME_SECONDS);
  fail_unless (range->min.seconds == 0.0);
  fail_unless (range->max.type == GST_RTSP_TIME_END);
  gst_rtsp_range_free (range);


  fail_unless (gst_rtsp_range_parse ("npt=1.123-", &range) == GST_RTSP_OK);
  fail_unless (range->unit == GST_RTSP_RANGE_NPT);
  fail_unless (range->min.type == GST_RTSP_TIME_SECONDS);
  fail_unless (range->min.seconds == 1.123);
  fail_unless (range->max.type == GST_RTSP_TIME_END);
  gst_rtsp_range_free (range);

  fail_unless (gst_rtsp_range_parse ("npt=10,20-20.10", &range) == GST_RTSP_OK);
  fail_unless (range->unit == GST_RTSP_RANGE_NPT);
  fail_unless (range->min.type == GST_RTSP_TIME_SECONDS);
  fail_unless (range->min.seconds == 10.20);
  fail_unless (range->max.type == GST_RTSP_TIME_SECONDS);
  fail_unless (range->max.seconds == 20.10);
  gst_rtsp_range_free (range);

  fail_unless (gst_rtsp_range_parse ("npt=500-15.001", &range) == GST_RTSP_OK);
  fail_unless (range->unit == GST_RTSP_RANGE_NPT);
  fail_unless (range->min.type == GST_RTSP_TIME_SECONDS);
  fail_unless (range->min.seconds == 500);
  fail_unless (range->max.type == GST_RTSP_TIME_SECONDS);
  fail_unless (range->max.seconds == 15.001);
  gst_rtsp_range_free (range);

  fail_unless (gst_rtsp_range_parse ("npt=20:34.23-",
          &range) == GST_RTSP_EINVAL);
  fail_unless (gst_rtsp_range_parse ("npt=10:20;34.23-",
          &range) == GST_RTSP_EINVAL);
  fail_unless (gst_rtsp_range_parse ("npt=0:4.23-", &range) == GST_RTSP_EINVAL);

  fail_unless (gst_rtsp_range_parse ("npt=20:12:34.23-21:45:00.01",
          &range) == GST_RTSP_OK);
  fail_unless (range->unit == GST_RTSP_RANGE_NPT);
  fail_unless (range->min.type == GST_RTSP_TIME_SECONDS);
  fail_unless (range->min.seconds == 72754.23);
  fail_unless (range->max.type == GST_RTSP_TIME_SECONDS);
  fail_unless (range->max.seconds == 78300.01);
  gst_rtsp_range_free (range);
}

GST_END_TEST;

GST_START_TEST (test_rtsp_range_smpte)
{
  GstRTSPTimeRange *range;

  fail_unless (gst_rtsp_range_parse ("smpte=", &range) == GST_RTSP_EINVAL);
  fail_unless (gst_rtsp_range_parse ("smpte=0", &range) == GST_RTSP_EINVAL);
  fail_unless (gst_rtsp_range_parse ("smpte=-", &range) == GST_RTSP_EINVAL);
  fail_unless (gst_rtsp_range_parse ("smpte=-12:09:34",
          &range) == GST_RTSP_EINVAL);
  fail_unless (gst_rtsp_range_parse ("smpte=12:09:34",
          &range) == GST_RTSP_EINVAL);

  fail_unless (gst_rtsp_range_parse ("smpte=00:00:00-", &range) == GST_RTSP_OK);
  fail_unless (range->unit == GST_RTSP_RANGE_SMPTE);
  fail_unless (range->min.type == GST_RTSP_TIME_FRAMES);
  fail_unless (range->min.seconds == 0.0);
  fail_unless (range->min.frames == 0.0);
  fail_unless (range->max.type == GST_RTSP_TIME_END);
  gst_rtsp_range_free (range);

  fail_unless (gst_rtsp_range_parse ("smpte=10:34:23-20:12:09:20.89",
          &range) == GST_RTSP_OK);
  fail_unless (range->unit == GST_RTSP_RANGE_SMPTE);
  fail_unless (range->min.type == GST_RTSP_TIME_FRAMES);
  fail_unless (range->min.seconds == 38063.0);
  fail_unless (range->min.frames == 0.0);
  fail_unless (range->max.type == GST_RTSP_TIME_FRAMES);
  fail_unless (range->max.seconds == 72729.0);
  fail_unless (range->max.frames == 20.89);
  gst_rtsp_range_free (range);
}

GST_END_TEST;


static Suite *
rtsp_suite (void)
{
  Suite *s = suite_create ("rtsp support library");
  TCase *tc_chain = tcase_create ("general");

  suite_add_tcase (s, tc_chain);
  tcase_add_test (tc_chain, test_rtsp_url_basic);
  tcase_add_test (tc_chain, test_rtsp_url_components_1);
  tcase_add_test (tc_chain, test_rtsp_url_components_2);
  tcase_add_test (tc_chain, test_rtsp_url_components_3);
  tcase_add_test (tc_chain, test_rtsp_range_npt);
  tcase_add_test (tc_chain, test_rtsp_range_smpte);

  return s;
}

int
main (int argc, char **argv)
{
  int nf;

  Suite *s = rtsp_suite ();
  SRunner *sr = srunner_create (s);

  gst_check_init (&argc, &argv);

  srunner_run_all (sr, CK_NORMAL);
  nf = srunner_ntests_failed (sr);
  srunner_free (sr);

  return nf;
}
