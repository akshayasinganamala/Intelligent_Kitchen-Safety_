from flask import Flask, request, jsonify, render_template
import time
import sqlite3
from datetime import datetime

app = Flask(__name__)

# ---------------- DATABASE ----------------
DB_NAME = "kitchen_safety.db"

def init_db():
    conn = sqlite3.connect(DB_NAME)
    c = conn.cursor()

    c.execute("""
        CREATE TABLE IF NOT EXISTS risk_events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT,
            temp REAL,
            gas REAL,
            person INTEGER,
            temp_rate REAL,
            gas_rate REAL,
            absence_time REAL,
            risk_score INTEGER,
            status TEXT
        )
    """)

    conn.commit()
    conn.close()

init_db()


def store_risk_event(data):
    conn = sqlite3.connect(DB_NAME)
    c = conn.cursor()

    c.execute("""
        INSERT INTO risk_events (
            timestamp, temp, gas, person,
            temp_rate, gas_rate, absence_time,
            risk_score, status
        )
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    """, (
        datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        data["temp"],
        data["gas"],
        data["person"],
        data["temp_rate"],
        data["gas_rate"],
        data["absence_time"],
        data["risk_score"],
        data["status"]
    ))

    conn.commit()
    conn.close()


# ---------------- GLOBAL DATA ----------------
latest_data = {
    "temp": 0,
    "gas": 0,
    "person": 1,
    "temp_rate": 0,
    "gas_rate": 0,
    "absence_time": 0,
    "risk_score": 0,
    "status": "SAFE",
    "risk": False
}

temp_history = []
gas_history = []
last_seen_time = time.time()

WINDOW = 15


# ---------------- UI PAGE ----------------
@app.route("/")
def dashboard():
    return render_template("index.html")


# ---------------- SENSOR UPDATE ----------------
@app.route("/update", methods=["POST"])
def update():
    global temp_history, gas_history, last_seen_time, latest_data

    data = request.get_json()
    if not data:
        return jsonify({"error": "No data"}), 400

    temp = float(data.get("temp", 0))
    gas = float(data.get("gas", 0))
    person = int(data.get("person", 1))

    now = time.time()

    # -------- HISTORY --------
    temp_history.append((now, temp))
    gas_history.append((now, gas))

    temp_history = [(t, v) for (t, v) in temp_history if now - t <= WINDOW]
    gas_history = [(t, v) for (t, v) in gas_history if now - t <= WINDOW]

    def rate(history):
        if len(history) < 2:
            return 0
        t0, v0 = history[0]
        t1, v1 = history[-1]
        return abs(v1 - v0) / max((t1 - t0), 0.1)

    temp_rate = rate(temp_history)
    gas_rate = rate(gas_history)

    # -------- ABSENCE --------
    if person == 1:
        last_seen_time = now

    absence_time = now - last_seen_time

    # -------- RISK ENGINE (IMPROVED THRESHOLDS) --------
    risk_score = 0

    # Temperature logic (more realistic)
    if temp >= 60:
        risk_score += 50
    elif temp >= 45:
        risk_score += 30

    # Gas logic (depends on calibration)
    if gas >= 300:
        risk_score += 50
    elif gas >= 150:
        risk_score += 30

    # Rate of change
    if temp_rate > 2:
        risk_score += 20

    if gas_rate > 100:
        risk_score += 20

    # Absence
    if absence_time > 30:
        risk_score += 30

    # -------- FINAL STATUS --------
    if risk_score >= 80:
        status = "DANGER"
        risk_flag = True
    elif risk_score >= 40:
        status = "WARNING"
        risk_flag = True
    else:
        status = "SAFE"
        risk_flag = False

    latest_data = {
        "temp": temp,
        "gas": gas,
        "person": person,
        "temp_rate": round(temp_rate, 2),
        "gas_rate": round(gas_rate, 2),
        "absence_time": round(absence_time, 2),
        "risk_score": risk_score,
        "status": status,
        "risk": risk_flag
    }

    # -------- STORE ONLY IMPORTANT EVENTS --------
    if status in ["WARNING", "DANGER"]:
        store_risk_event(latest_data)

    return jsonify(latest_data)


# ---------------- GET LIVE DATA ----------------
@app.route("/get")
def get():
    return jsonify(latest_data)


# ---------------- MONTHLY REPORT ----------------
@app.route("/monthly-report")
def monthly_report():
    conn = sqlite3.connect(DB_NAME)
    c = conn.cursor()

    c.execute("""
        SELECT * FROM risk_events
        WHERE strftime('%Y-%m', timestamp) = strftime('%Y-%m', 'now')
    """)

    rows = c.fetchall()
    conn.close()

    return jsonify(rows)


# ---------------- ALL DATA (DEBUG) ----------------
@app.route("/all-data")
def all_data():
    conn = sqlite3.connect(DB_NAME)
    c = conn.cursor()

    c.execute("SELECT * FROM risk_events ORDER BY id DESC")
    rows = c.fetchall()

    conn.close()
    return jsonify(rows)


# ---------------- RUN ----------------
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)