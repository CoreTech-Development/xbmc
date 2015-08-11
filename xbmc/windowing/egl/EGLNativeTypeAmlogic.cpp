/*
 *      Copyright (C) 2011-2013 Team XBMC
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

#include "EGLNativeTypeAmlogic.h"
#include "guilib/gui3d.h"
#include "utils/AMLUtils.h"
#include "utils/StringUtils.h"
#include "utils/SysfsUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <EGL/egl.h>

CEGLNativeTypeAmlogic::CEGLNativeTypeAmlogic()
{
  const char *env_framebuffer = getenv("FRAMEBUFFER");

  // default to framebuffer 0
  m_framebuffer_name = "fb0";
  if (env_framebuffer)
  {
    std::string framebuffer(env_framebuffer);
    std::string::size_type start = framebuffer.find("fb");
    m_framebuffer_name = framebuffer.substr(start);
  }
  m_nativeWindow = NULL;
}

CEGLNativeTypeAmlogic::~CEGLNativeTypeAmlogic()
{
}

bool CEGLNativeTypeAmlogic::CheckCompatibility()
{
  std::string name;
  std::string modalias = "/sys/class/graphics/" + m_framebuffer_name + "/device/modalias";

  SysfsUtils::GetString(modalias, name);
  StringUtils::Trim(name);
  if (name == "platform:mesonfb")
    return true;
  return false;
}

void CEGLNativeTypeAmlogic::Initialize()
{
  aml_permissions();
  aml_cpufreq_min(true);
  aml_cpufreq_max(true);
  return;
}
void CEGLNativeTypeAmlogic::Destroy()
{
  aml_cpufreq_min(false);
  aml_cpufreq_max(false);
  return;
}

bool CEGLNativeTypeAmlogic::CreateNativeDisplay()
{
  m_nativeDisplay = EGL_DEFAULT_DISPLAY;
  return true;
}

bool CEGLNativeTypeAmlogic::CreateNativeWindow()
{
#if defined(_FBDEV_WINDOW_H_)
  fbdev_window *nativeWindow = new fbdev_window;
  if (!nativeWindow)
    return false;

  if (aml_get_device_type() <= AML_DEVICE_TYPE_M3) {
    nativeWindow->width = 1280;
    nativeWindow->height = 720;
  } else {
    nativeWindow->width = 1920;
    nativeWindow->height = 1080;
  }
  m_nativeWindow = nativeWindow;

  if (aml_get_device_type() > AML_DEVICE_TYPE_M3) {
    SetFramebufferResolution(nativeWindow->width, nativeWindow->height);
  }

  return true;
#else
  return false;
#endif
}

bool CEGLNativeTypeAmlogic::GetNativeDisplay(XBNativeDisplayType **nativeDisplay) const
{
  if (!nativeDisplay)
    return false;
  *nativeDisplay = (XBNativeDisplayType*) &m_nativeDisplay;
  return true;
}

bool CEGLNativeTypeAmlogic::GetNativeWindow(XBNativeWindowType **nativeWindow) const
{
  if (!nativeWindow)
    return false;
  *nativeWindow = (XBNativeWindowType*) &m_nativeWindow;
  return true;
}

bool CEGLNativeTypeAmlogic::DestroyNativeDisplay()
{
  return true;
}

bool CEGLNativeTypeAmlogic::DestroyNativeWindow()
{
#if defined(_FBDEV_WINDOW_H_)
  delete (fbdev_window*)m_nativeWindow, m_nativeWindow = NULL;
#endif
  return true;
}

bool CEGLNativeTypeAmlogic::GetNativeResolution(RESOLUTION_INFO *res) const
{
  std::string mode;
  SysfsUtils::GetString("/sys/class/display/mode", mode);
  return aml_mode_to_resolution(mode.c_str(), res);
}

bool CEGLNativeTypeAmlogic::SetNativeResolution(const RESOLUTION_INFO &res)
{
#if defined(_FBDEV_WINDOW_H_)
  if ((m_nativeWindow) && (aml_get_device_type() > AML_DEVICE_TYPE_M3))
  {
    ((fbdev_window *)m_nativeWindow)->width = res.iScreenWidth;
    ((fbdev_window *)m_nativeWindow)->height = res.iScreenHeight;
  }
#endif

  switch((int)(0.5 + res.fRefreshRate))
  {
    default:
    case 60:
      switch(res.iScreenWidth)
      {
        case 720:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            SetDisplayResolution("480i");
          else
            SetDisplayResolution("480p");
          break;
        default:
        case 1280:
          SetDisplayResolution("720p");
          break;
        case 1920:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            SetDisplayResolution("1080i");
          else
            SetDisplayResolution("1080p");
          break;
      }
      break;
    case 59:
      switch(res.iScreenWidth)
      {
        default:
        case 1280:
          SetDisplayResolution("720p59hz");
          break;
        case 1920:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            SetDisplayResolution("1080i59hz");
          else
            SetDisplayResolution("1080p59hz");
          break;
      }
      break;
    case 50:
      switch(res.iScreenWidth)
      {
        case 720:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            SetDisplayResolution("576i");
          else
            SetDisplayResolution("576p");
          break;
        default:
        case 1280:
          SetDisplayResolution("720p50hz");
          break;
        case 1920:
          if (res.dwFlags & D3DPRESENTFLAG_INTERLACED)
            SetDisplayResolution("1080i50hz");
          else
            SetDisplayResolution("1080p50hz");
          break;
      }
      break;
    case 30:
      switch(res.iScreenWidth)
      {
        default:
        case 1920:
          SetDisplayResolution("1080p30hz");
          break;
        case 3840:
          SetDisplayResolution("4k2k30hz");
          break;
      }
      break;
    case 29:
      switch(res.iScreenWidth)
      {
        default:
        case 1920:
          SetDisplayResolution("1080p29hz");
          break;
        case 3840:
          SetDisplayResolution("4k2k29hz");
          break;
      }
      break;
    case 25:
      switch(res.iScreenWidth)
      {
        default:
        case 1920:
          SetDisplayResolution("1080p25hz");
          break;
        case 3840:
          SetDisplayResolution("4k2k25hz");
          break;
      }
      break;
    case 24:
      switch(res.iScreenWidth)
      {
        default:
        case 1920:
          SetDisplayResolution("1080p24hz");
          break;
        case 3840:
          SetDisplayResolution("4k2k24hz");
          break;
        case 4096:
          SetDisplayResolution("4k2ksmpte");
          break;
      }
      break;
    case 23:
      switch(res.iScreenWidth)
      {
        default:
        case 1920:
          SetDisplayResolution("1080p23hz");
          break;
        case 3840:
          SetDisplayResolution("4k2k23hz");
          break;
      }
      break;
  }

  return true;
}

bool CEGLNativeTypeAmlogic::ProbeResolutions(std::vector<RESOLUTION_INFO> &resolutions)
{
  std::vector<std::string> probe_str;
  if (IsHdmiConnected())
  {
    std::string valstr;
    SysfsUtils::GetString("/sys/class/amhdmitx/amhdmitx0/disp_cap", valstr);
    probe_str = StringUtils::Split(valstr, "\n");
  }
  else
  {
    probe_str.push_back("480i");
    probe_str.push_back("480p");
    probe_str.push_back("576i");
    probe_str.push_back("576p");
  }

  resolutions.clear();
  RESOLUTION_INFO res;
  for (std::vector<std::string>::const_iterator i = probe_str.begin(); i != probe_str.end(); ++i)
  {
    if(aml_mode_to_resolution(i->c_str(), &res))
      resolutions.push_back(res);
  }
  return resolutions.size() > 0;

}

bool CEGLNativeTypeAmlogic::GetPreferredResolution(RESOLUTION_INFO *res) const
{
  // check display/mode, it gets defaulted at boot
  if (!GetNativeResolution(res))
  {
    // punt to 720p or 480cvbs if we get nothing
    if (IsHdmiConnected())
      aml_mode_to_resolution("720p", res);
    else
      aml_mode_to_resolution("480p", res);
  }

  return true;
}

bool CEGLNativeTypeAmlogic::ShowWindow(bool show)
{
  std::string blank_framebuffer = "/sys/class/graphics/" + m_framebuffer_name + "/blank";
  SysfsUtils::SetInt(blank_framebuffer.c_str(), show ? 0 : 1);
  return true;
}

bool CEGLNativeTypeAmlogic::SetDisplayResolution(const char *resolution)
{
  std::string mode = resolution;
  // switch display resolution
  SysfsUtils::SetString("/sys/class/display/mode", mode.c_str());

#ifdef TARGET_ANDROID
  SetupVideoScaling(mode.c_str());
#else
  DisableFreeScale();

  if (aml_get_device_type() > AML_DEVICE_TYPE_M3)
  {
    RESOLUTION_INFO res;
    aml_mode_to_resolution(mode.c_str(), &res);
    SetFramebufferResolution(res);
  }

  if ((StringUtils::StartsWith(mode, "4k2k")) || ((StringUtils::StartsWith(mode, "1080")) && (aml_get_device_type() <= AML_DEVICE_TYPE_M3)))
  {
    EnableFreeScale();
  }
#endif

  return true;
}

void CEGLNativeTypeAmlogic::SetupVideoScaling(const char *mode)
{
  SysfsUtils::SetInt("/sys/class/graphics/fb0/blank",      1);
  SysfsUtils::SetInt("/sys/class/graphics/fb0/free_scale", 0);
  SysfsUtils::SetInt("/sys/class/graphics/fb1/free_scale", 0);
  SysfsUtils::SetInt("/sys/class/ppmgr/ppscaler",          0);

  if (strstr(mode, "1080"))
  {
    SysfsUtils::SetString("/sys/class/graphics/fb0/request2XScale", "8");
    SysfsUtils::SetString("/sys/class/graphics/fb1/scale_axis",     "1280 720 1920 1080");
    SysfsUtils::SetString("/sys/class/graphics/fb1/scale",          "0x10001");
  }
  else
  {
    SysfsUtils::SetString("/sys/class/graphics/fb0/request2XScale", "16 1280 720");
  }

  SysfsUtils::SetInt("/sys/class/graphics/fb0/blank", 0);
}

void CEGLNativeTypeAmlogic::EnableFreeScale()
{
  RESOLUTION_INFO res;
  std::string mode;
  SysfsUtils::GetString("/sys/class/display/mode", mode);
  aml_mode_to_resolution(mode.c_str(), &res);

  char fsaxis_str[256] = {0};
  sprintf(fsaxis_str, "0 0 %d %d", res.iWidth-1, res.iHeight-1);
  char waxis_str[256] = {0};
  sprintf(waxis_str, "0 0 %d %d", res.iScreenWidth-1, res.iScreenHeight-1);

  SysfsUtils::SetInt("/sys/class/video/disable_video", 1);
  SysfsUtils::SetString("/sys/class/graphics/fb0/free_scale_axis", fsaxis_str);
  SysfsUtils::SetString("/sys/class/graphics/fb0/window_axis", waxis_str);
  SysfsUtils::SetString("/sys/class/video/axis", waxis_str);
  SysfsUtils::SetInt("/sys/class/graphics/fb0/free_scale", 0x10001);
  SysfsUtils::SetInt("/sys/class/video/disable_video", 2);
}

void CEGLNativeTypeAmlogic::DisableFreeScale()
{
  SysfsUtils::SetInt("/sys/class/video/disable_video", 1);
  SysfsUtils::SetString("/sys/class/graphics/fb0/free_scale_axis", "0 0 0 0");
  SysfsUtils::SetString("/sys/class/graphics/fb0/window_axis", "0 0 0 0");
  SysfsUtils::SetString("/sys/class/video/axis", "0 0 0 0");
  SysfsUtils::SetInt("/sys/class/graphics/fb0/free_scale", 0);
  SysfsUtils::SetInt("/sys/class/video/disable_video", 0);
}

void CEGLNativeTypeAmlogic::SetFramebufferResolution(const RESOLUTION_INFO &res) const
{
  std::string mode;
  SysfsUtils::GetString("/sys/class/display/mode", mode);

  if (StringUtils::StartsWith(mode, "4k2k"))
  {
    SetFramebufferResolution(res.iWidth, res.iHeight);
  } else {
    SetFramebufferResolution(res.iScreenWidth, res.iScreenHeight);
  }
}

void CEGLNativeTypeAmlogic::SetFramebufferResolution(int width, int height) const
{
  int fd0;
  std::string framebuffer = "/dev/" + m_framebuffer_name;

  if ((fd0 = open(framebuffer.c_str(), O_RDWR)) >= 0)
  {
    struct fb_var_screeninfo vinfo;
    if (ioctl(fd0, FBIOGET_VSCREENINFO, &vinfo) == 0)
    {
      vinfo.xres = width;
      vinfo.yres = height;
      vinfo.xres_virtual = 1920;
      vinfo.yres_virtual = 2160;
      vinfo.bits_per_pixel = 32;
      vinfo.activate = FB_ACTIVATE_ALL;
      ioctl(fd0, FBIOPUT_VSCREENINFO, &vinfo);
    }
    close(fd0);
  }
}

bool CEGLNativeTypeAmlogic::IsHdmiConnected() const
{
  int hpd_state;
  SysfsUtils::GetInt("/sys/class/amhdmitx/amhdmitx0/hpd_state", hpd_state);
  if(hpd_state == 2);
  {
    return 1;
  }
  return 0;
}
