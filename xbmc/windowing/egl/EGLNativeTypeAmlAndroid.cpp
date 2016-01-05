/*
 *      Copyright (C) 2011-2014 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "system.h"
#include <EGL/egl.h>
#include "EGLNativeTypeAmlAndroid.h"
#include "utils/log.h"
#include "guilib/gui3d.h"
#include "android/activity/XBMCApp.h"
#include "utils/StringUtils.h"
#include "utils/SysfsUtils.h"
#include "utils/AMLUtils.h"

bool CEGLNativeTypeAmlAndroid::m_isWritable = false;

bool CEGLNativeTypeAmlAndroid::CheckCompatibility()
{
  if (aml_present())
  {
    m_isWritable = false;
    if (SysfsUtils::HasRW("/sys/class/display/mode"))
      m_isWritable = true;
    else
      CLog::Log(LOGINFO, "AMLEGL: no rw on /sys/class/display/mode");
    return true;
  }
  return false;
}

bool CEGLNativeTypeAmlAndroid::GetNativeResolution(RESOLUTION_INFO *res) const
{
  CEGLNativeTypeAndroid::GetNativeResolution(&m_fb_res);

  std::string mode;
  RESOLUTION_INFO hdmi_res;
  if (SysfsUtils::GetString("/sys/class/display/mode", mode) == 0 && aml_mode_to_resolution(mode.c_str(), &hdmi_res))
  {
    m_curHdmiResolution = mode;
    *res = hdmi_res;
    res->iWidth = m_fb_res.iWidth;
    res->iHeight = m_fb_res.iHeight;
    res->iSubtitles = (int)(0.965 * res->iHeight);
  }
  else
    *res = m_fb_res;

  return true;
}

bool CEGLNativeTypeAmlAndroid::SetNativeResolution(const RESOLUTION_INFO &res)
{
  if (!m_isWritable)
    return false;

  switch((int)(res.fRefreshRate*10))
  {
    default:
    case 600:
      switch(res.iScreenWidth)
      {
        if (aml_get_device_type() > AML_DEVICE_TYPE_M8M2)
        {
          case 4096:
            return SetDisplayResolution("smpte60hz");
            break;
          case 3840:
            return SetDisplayResolution("2160p60hz");
            break;
        }
        case 1920:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            if (aml_get_device_type() > AML_DEVICE_TYPE_M8M2)
              return SetDisplayResolution("1080i60hz");
            else
              return SetDisplayResolution("1080i");
          else
            if (aml_get_device_type() > AML_DEVICE_TYPE_M8M2)
              return SetDisplayResolution("1080p60hz");
            else
              return SetDisplayResolution("1080p");
          break;
        default:
        case 1280:
          if (aml_get_device_type() > AML_DEVICE_TYPE_M8M2)
            return SetDisplayResolution("720p60hz");
          else
            return SetDisplayResolution("720p");
          break;
        case 720:
          if (!aml_IsHdmiConnected())
            return SetDisplayResolution("480cvbs");
          else if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            return SetDisplayResolution("480i");
          else
            return SetDisplayResolution("480p");
          break;
      }
      break;
    case 599:
      switch(res.iScreenWidth)
      {
        if (aml_get_device_type() == AML_DEVICE_TYPE_M9B)
        {
          case 4096:
            return SetDisplayResolution("smpte59hz");
            break;
          case 3840:
            return SetDisplayResolution("2160p59hz");
            break;
        }
        case 1920:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            return SetDisplayResolution("1080i59hz");
          else
            return SetDisplayResolution("1080p59hz");
          break;
        default:
          return SetDisplayResolution("720p59hz");
          break;
      }
      break;
    case 500:
      switch(res.iScreenWidth)
      {
        if (aml_get_device_type() > AML_DEVICE_TYPE_M8M2)
        {
          case 4096:
            return SetDisplayResolution("smpte50hz");
            break;
          case 3840:
            return SetDisplayResolution("2160p50hz");
            break;
        }
        case 1920:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            return SetDisplayResolution("1080i50hz");
          else
            return SetDisplayResolution("1080p50hz");
          break;
        default:
        case 1280:
          return SetDisplayResolution("720p50hz");
          break;
        case 720:
          if (!aml_IsHdmiConnected())
            return SetDisplayResolution("576cvbs");
          else if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            return SetDisplayResolution("576i");
          else
            return SetDisplayResolution("576p");
          break;
      }
      break;
    case 300:
      switch(res.iScreenWidth)
      {
        case 3840:
          if (aml_get_device_type() > AML_DEVICE_TYPE_M8M2)
            return SetDisplayResolution("2160p30hz");
          else
            return SetDisplayResolution("4k2k30hz");
          break;
        default:
          return SetDisplayResolution("1080p30hz");
          break;
      }
      break;
    case 299:
      switch(res.iScreenWidth)
      {
        case 3840:
          if (aml_get_device_type() > AML_DEVICE_TYPE_M8M2)
            return SetDisplayResolution("2160p29hz");
          else
            return SetDisplayResolution("4k2k29hz");
          break;
        default:
          return SetDisplayResolution("1080p29hz");
          break;
      }
      break;
    case 250:
      switch(res.iScreenWidth)
      {
        case 3840:
          if (aml_get_device_type() > AML_DEVICE_TYPE_M8M2)
            return SetDisplayResolution("2160p25hz");
          else
            return SetDisplayResolution("4k2k25hz");
          break;
        default:
          return SetDisplayResolution("1080p25hz");
          break;
      }
      break;
    case 240:
      switch(res.iScreenWidth)
      {
        case 4096:
          if (aml_get_device_type() > AML_DEVICE_TYPE_M8M2)
            return SetDisplayResolution("smpte24hz");
          else
            return SetDisplayResolution("4k2ksmpte");
          break;
        case 3840:
          if (aml_get_device_type() > AML_DEVICE_TYPE_M8M2)
            return SetDisplayResolution("2160p24hz");
          else
            return SetDisplayResolution("4k2k24hz");
          break;
        default:
          return SetDisplayResolution("1080p24hz");
          break;
      }
      break;
    case 239:
      switch(res.iScreenWidth)
      {
        case 4096:
          return SetDisplayResolution("smpte23hz");
          break;
        case 3840:
          if (aml_get_device_type() > AML_DEVICE_TYPE_M8M2)
            return SetDisplayResolution("2160p23hz");
          else
            return SetDisplayResolution("4k2k23hz");
          break;
        default:
          return SetDisplayResolution("1080p23hz");
          break;
      }
      break;
  }

  return false;
}

bool CEGLNativeTypeAmlAndroid::ProbeResolutions(std::vector<RESOLUTION_INFO> &resolutions)
{
  CEGLNativeTypeAndroid::GetNativeResolution(&m_fb_res);

  std::string valstr;
  if (SysfsUtils::GetString("/sys/class/amhdmitx/amhdmitx0/disp_cap", valstr) < 0)
    return false;
  std::vector<std::string> probe_str = StringUtils::Split(valstr, "\n");

  resolutions.clear();
  RESOLUTION_INFO res;
  for (size_t i = 0; i < probe_str.size(); i++)
  {
    if(aml_mode_to_resolution(probe_str[i].c_str(), &res))
    {
      res.iWidth = m_fb_res.iWidth;
      res.iHeight = m_fb_res.iHeight;
      res.iSubtitles = (int)(0.965 * res.iHeight);
      resolutions.push_back(res);
    }
  }
  return resolutions.size() > 0;

}

bool CEGLNativeTypeAmlAndroid::GetPreferredResolution(RESOLUTION_INFO *res) const
{
  return GetNativeResolution(res);
}

bool CEGLNativeTypeAmlAndroid::SetDisplayResolution(const char *resolution)
{
  if (m_curHdmiResolution == resolution)
    return true;

  // switch display resolution
  if (SysfsUtils::SetString("/sys/class/display/mode", resolution) < 0)
    return false;

  m_curHdmiResolution = resolution;

  return true;
}

