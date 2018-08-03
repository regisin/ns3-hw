/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) University of Washington
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
 */

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <linux/wireless.h>

#include <cstdio>
#include <cstring>
#include <string>

#include "creator-utils.h"

int
main (int argc, char *argv[])
{
  int power;
  std::string deviceName;
  int c;
  opterr = 0;
  while ((c = getopt (argc, argv, "p:d:")) != -1)
  {
    switch (c)
      {
      case 'd':
        deviceName = optarg;
        break;
      case 'p':
        power = std::stoi(optarg);
        break;
      }
  }

  int sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
  ABORT_IF (sockfd == -1, "could not create socket", -1);
  struct iwreq request;
  memset(&request, 0, sizeof(request));
  strncpy(request.ifr_name, deviceName.c_str(), IFNAMSIZ);
  request.u.txpower.value = power;
  if (ioctl(sockfd, SIOCSIWTXPOW, &request) != -1)
  {
      // LOG("Tx-power is: %d dBm\n" << request.u.txpower.value);
  }else{
      ABORT_IF (sockfd == -1, "could not perform ioctl", -1);
      return -1;
  }
  return 0;
}

