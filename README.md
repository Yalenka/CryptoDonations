# CryptoDonations – UE5 Plugin

Simple Unreal Engine 5 plugin that lets players **donate cryptocurrency** to your project using the **NowPayments** API.

## Features (current – early 2026)

- Create payment links / invoices via NowPayments
- Basic HTTP communication: UE5 → your backend
- Blueprint-friendly node: `CreatePayment`

**Known limitations**

- Donation / currency state lives only in Widget → lost on reload / level change
- **Not multiplayer safe** (no replication)
- Basic error handling
- No payment status polling or webhook support yet

**Recommended improvement**  
Move important logic & state to **PlayerState** (or GameState for global tracking).

## Requirements

- Unreal Engine 5.3 / 5.4 (tested up to 5.4)
- Node.js 20.x for the backend
- NowPayments merchant account (API key + IPN secret)
- Linux or WSL2 (Windows) to run the backend

## Installation & Setup

### 1. Install the Plugin in Your UE5 Project

1. Download or clone this repository
2. Copy the `CryptoDonations` folder into your project's `Plugins/` directory  
   → final path: `YourProject/Plugins/CryptoDonations/`
3. Open your project → **Edit → Plugins** → search "CryptoDonations" → enable it
4. Restart the Unreal Editor

### 2. Backend Setup (Node.js server)

#### Folder structure
