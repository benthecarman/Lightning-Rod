# User Guide

- [User Guide](#user-guide)
  - [Lightning Rods](#lightning-rods)
    - [Setup](#setup)
    - [Optional Features](#optional-features)
      - [Blocking IPs](#blocking-ips)
      - [Blocking default RPC commands](#blocking-default-rpc-commands)
      - [Allowing RPC commands](#allowing-rpc-commands)
  - [Sparks](#sparks)
    - [Finding a Lightning Rod](#finding-a-lightning-rod)
    - [c-lightning](#c-lightning)
    - [lnd](#lnd)

## Lightning Rods

A Lightning Rod is a program that you will run on top of your Bitcoin Core node. Using a Lightning Rod will not give you any additional benefits but it will enable many other users.  A Lightning Rod will allow a user to run an instance of lnd or c-lightning without needing to run a full node because they can connect to your Lighting Rod.

For a Lightning Rod to opperate correctly you must have either a public IP address or have the Ports you are broadcasting to forwarded (defaults: 8331, 28330, 28831)

### Setup

To correctly setup your Lightning Rod your Bitcoin Core node must be confiured correctly, a sample `bitcoin.conf` file can be found [here](sample_bitcoin.conf), using this should allow your Lightning Rod to run without any configuration, however, it is **HIGHLY** recommended to change the rpcuser and rpcpassword from the defaults, as it could leave your node vulunarble to attacks.

For further configuration options a sample config file can be found [here](sample.conf).

### Optional Features

#### Blocking IPs

An IP address can be blocked from connecting to your node by adding
`blacklistip=<ip>` to your config file or using `--blacklistip=<ip>`
as a command line argument. This option can be specified multiple times.

IP addresses of peers can be found in your log files.

#### Blocking default RPC commands

By default a Lighting Rod will only allow the following RPC commands to be passed to your bitcoin node:

- echo
- getblockhash
- getblock
- estimatesmartfee
- getbestblockhash
- sendrawtransaction
- getblockcount
- getnetworkinfo
- getblockchaininfo
- getblockheader
- gettxout

Any of these commands can be blacklisted by adding `blacklistcmd=<cmd>`
to your config file or using `--blacklistcmd=<cmd>` as a command line argument. This option can be specified multiple times.

#### Allowing RPC commands

A command can added to the list of allowed RPC commands by adding `whitelistcmd=<cmd>` to your config file or using `--whitelistcmd=<cmd>` as a command line argument. This option can be specified multiple times.

## Sparks

A spark is a user that is not running a Bitcoin full node but still wants to access the lightning network. A spark is able to access the lightning network by connecting their lightning node to a Lightning Rod instead of their own full node. Below are guides on how to connect to a Lighting Rod using [c-lightning](#c-lightning) and [lnd](#lnd).

### Finding a Lightning Rod

To find a list of trusted Lightning Rods vist our [website](https://lightning-rod.net) and view the trusted hosts page.  You can also try and find others that are not listed through twitter, reddit, IRC, or any other public forum.

### c-lightning

Note: You must find a Lighting Rod to connect to first.

To connect to a Lightning Rod using c-lightning you must configure c-lightning correctly. You can find a sample c-lightning config file [here](sample_c-lightning_config).

### lnd

Note: You must find a Lighting Rod, with ZMQ enabled, to connect to first.

To connect to a Lightning Rod using lnd you must configure lnd correctly. You can find a sample lnd config file [here](sample_lnd.conf).
