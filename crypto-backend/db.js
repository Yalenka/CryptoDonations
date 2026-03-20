const sqlite3 = require("sqlite3").verbose();

const db = new sqlite3.Database("./payments.db");

db.serialize(() => {
  db.run(`
    CREATE TABLE IF NOT EXISTS payments (
      id TEXT PRIMARY KEY,
      userId TEXT,
      status TEXT,
      reward INTEGER,
      createdAt DATETIME DEFAULT CURRENT_TIMESTAMP
    )
  `);
});

module.exports = db;
