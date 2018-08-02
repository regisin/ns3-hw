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

#ifndef PI_NET_DEVICE_H
#define PI_NET_DEVICE_H

#include "fd-net-device.h"

#include <utility>
#include <queue>

namespace ns3 {

class PiNetDevice : public FdNetDevice
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);


  /**
   * Constructor for the PiNetDevice.
   */
  PiNetDevice ();

  /**
   * Destructor for the PiNetDevice.
   */
  virtual ~PiNetDevice ();

  // PiNetDevice stuff
  /**
   * Get the tx-power value from the wireless NIC. Returns in dBm.
   *
   */
  int GetTxPower (void);

  /**
   * Set the tx-power for the wireless NIC.
   *
   * @param power the tx-power in dBm (32 is the maximum for Raspberry Pi 3 Mobel B).
   */
  void SetTxPower (int power);

};

} // namespace ns3

#endif /* PI_NET_DEVICE_H */

