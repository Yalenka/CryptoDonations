# CryptoDonations
A Simple Crypto Donation plugin!

A Simple plugin uses NowPayments API to enable your project to receive Crypto Donations!

## Usage:
### Open file .env and Just update these with real data and run the backend

PORT=3000
NOWPAYMENTS_API_KEY= your_real_crypto_wallet_api
NOWPAYMENTS_IPN_SECRET= your_real_crypto_wallet_ipn

### Run and Test locally
Open powershell and navigate to the server folder and run server.json
Open second powershell and run this command
```
Invoke-RestMethod -Uri "http://localhost:3000/create-payment" `
>> -Method POST `
>> -Headers @{ "Content-Type" = "application/json" } `
>> -Body '{"amount":100,"currency":"eth","userId":"test123"}'
```
You should see the response in the local Server
You should see another response online in your wallet at :: account.nowpayments.io/payments

## Expose the plugin API to blueprints to use it in the UMG or code your UMG in C++ to direct interact with plugin API

That's all!
