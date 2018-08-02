/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 INRIA, 2012 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alina Quereilhac <alina.quereilhac@inria.fr>
 *         Claudio Freire <klaussfreire@sourceforge.net>
 */

#include "pi-net-device.h"

#include "ns3/log.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <linux/wireless.h>
#include <cstdio>
#include <cstring>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PiNetDevice");
NS_OBJECT_ENSURE_REGISTERED (PiNetDevice);

TypeId
FdNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PiNetDevice")
    .SetParent<NetDevice> ()
    .SetGroupName ("PiNetDevice")
    .AddConstructor<PiNetDevice> ()
  ;
  return tid;
}

PiNetDevice::PiNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

PiNetDevice::~PiNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

int
PiNetDevice::GetTxPower (void)
{
  struct iwreq request;
  memset(&request, 0, sizeof(request));
  strncpy(request.ifr_name, "wlxc04a00128ba2", IFNAMSIZ);

  if (ioctl(m_fd, SIOCGIWTXPOW, &request) != -1)
  {
      NS_LOG_INFO("Tx-power is: " << request.u.txpower.value << " dBm");
      return request.u.txpower.value;
  }else{
      NS_LOG_ERROR("Unable to get tx-power using ioctl");
      return -1;
  }
}

void
PiNetDevice::SetTxPower (int power)
{
  struct iwreq request;
  memset(&request, 0, sizeof(request));
  strncpy(request.ifr_name, "wlxc04a00128ba2", IFNAMSIZ);
  request.u.txpower.value = power;

  if (ioctl(m_fd, SIOCSIWTXPOW, &request) != -1)
  {
      NS_LOG_INFO("Tx-power is: " << request.u.txpower.value << " dBm");
  }else{
      NS_LOG_ERROR("Unable to set tx-power using ioctl");
  }
}


} // namespace ns3
