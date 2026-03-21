import express from "express";
import axios from "axios";
import crypto from "crypto";
import { db } from "./db";

const app = express();
app.use(express.json());

const PORT = process.env.PORT || 3000;

// ===============================
// Reward calculation
// ===============================
function calculateReward(amount: number) {
  return Math.floor(amount * 120);
}

// ===============================
// CREATE PAYMENT
// ===============================
app.post("/create-payment", async (req, res) => {
  try {
    const { amount, currency, userId } = req.body;

    if (!amount || !currency || !userId) {
      return res.status(400).json({ error: "Missing params" });
    }

    const response = await axios.post(
      "https://api.nowpayments.io/v1/payment",
      {
        price_amount: amount,
        price_currency: "usd",
        pay_currency: currency.toLowerCase(),
      },
      {
        headers: {
          "x-api-key": process.env.NOWPAYMENTS_API_KEY,
        },
      },
    );

    const data = response.data;
    const reward = calculateReward(amount);

    // Save to DB
    db.run(
      `INSERT INTO payments (id, userId, status, reward) VALUES (?, ?, ?, ?)`,
      [data.payment_id, userId, "waiting", reward],
    );

    // Logging
    console.log("New payment created:");
    console.log("ID:", data.payment_id);
    console.log("User:", userId);
    console.log("USD:", amount);
    console.log("Crypto:", data.pay_amount, currency);
    console.log("Address:", data.pay_address);
    console.log("------------");

    res.json({
      payment_id: data.payment_id,
      pay_address: data.pay_address,
      pay_amount: data.pay_amount,
      qr_code_url: data.qr_code_url,
    });
  } catch (err) {
    const apiError = err.response?.data;

    console.error("PAYMENT ERROR:");
    console.error(apiError || err.message);

    // Handle minimum amount error cleanly
    if (apiError?.code === "AMOUNT_MINIMAL_ERROR") {
      return res.status(400).json({
        error: "Amount too low for selected cryptocurrency",
        details: apiError.message,
      });
    }

    res.status(500).json({
      error: "Payment creation failed",
      details: apiError,
    });
  }
});

// ===============================
// WEBHOOK (SECURE)
// ===============================
app.post("/webhook", (req, res) => {
  try {
    const signature = req.headers["x-nowpayments-sig"];
    const secret = process.env.NOWPAYMENTS_IPN_SECRET;
    if (!secret) {
      // ...
      return;
    }

    const hmac = crypto
      .createHmac("sha512", secret)
      .update(JSON.stringify(req.body))
      .digest("hex");

    if (hmac !== signature) {
      console.log("Invalid webhook signature");
      return res.sendStatus(403);
    }

    const payment = req.body;

    if (payment.payment_status === "finished") {
      db.get(
        `SELECT * FROM payments WHERE id = ?`,
        [payment.payment_id],
        (err, row) => {
          if (!row) {
            console.log("⚠️ Payment not found:", payment.payment_id);
            return;
          }

          // Prevent duplicate rewards
          if (row.status === "finished") {
            console.log("⚠️ Already processed:", payment.payment_id);
            return;
          }

          db.run(`UPDATE payments SET status = ? WHERE id = ?`, [
            "finished",
            payment.payment_id,
          ]);

          console.log("Payment confirmed:");
          console.log("ID:", payment.payment_id);
          console.log("User:", row.userId);
          console.log("Reward:", row.reward);
          console.log("------------");
        },
      );
    }

    res.sendStatus(200);
  } catch (err) {
    console.error("WEBHOOK ERROR:", err);
    res.sendStatus(500);
  }
});

// ===============================
// CHECK STATUS
// ===============================
app.get("/payment-status/:id", (req, res) => {
  db.get(
    `SELECT status, reward FROM payments WHERE id = ?`,
    [req.params.id],
    (err, row) => {
      if (!row) {
        return res.json({ status: "unknown" });
      }

      res.json(row);
    },
  );
});

// ===============================
app.listen(PORT, () => {
  console.log(`Server running on port ${PORT}`);
});
