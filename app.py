from flask import Flask, request, jsonify, render_template
import time

app = Flask(__name__)

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

    # ---------------- HISTORY ----------------
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

    # ---------------- ABSENCE ----------------
    if person == 1:
        last_seen_time = now

    absence_time = now - last_seen_time

    # ---------------- IMPROVED RISK ENGINE ----------------
    risk_score = 0

    # Basic conditions
    if temp >= 25:
        risk_score += 30

    if gas >= 10:
        risk_score += 30

    # Rate conditions (fast change = danger)
    if temp_rate > 1.5:
        risk_score += 20

    if gas_rate > 80:
        risk_score += 20

    # Absence = critical factor
    if person == 0:
        if absence_time > 10:
            risk_score += 20
        if absence_time > 30:
            risk_score += 40

    # Presence reduces risk
    if person == 1:
        risk_score -= 30

    # Clamp
    risk_score = max(risk_score, 0)

    # ---------------- STATUS ----------------
    if risk_score >= 70:
        status = "DANGER"
        risk_flag = True
    elif risk_score >= 40:
        status = "WARNING"
        risk_flag = True
    else:
        status = "SAFE"
        risk_flag = False

    # ---------------- UPDATE DATA ----------------
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

    return jsonify(latest_data)


# ---------------- API FOR UI ----------------
@app.route("/get")
def get():
    return jsonify(latest_data)


# ---------------- RUN ----------------
if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)