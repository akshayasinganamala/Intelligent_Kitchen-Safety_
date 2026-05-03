

A Flask-based kitchen environment dashboard that monitors temperature, gas level, human presence, and risk score. The dashboard fetches live sensor data from the server and also displays stored event history from a local SQLite database.

## Project Overview

This project provides a complete IoT safety monitoring workflow from sensor data capture to web visualization. It is designed as a proof-of-concept kitchen safety monitor that can receive live sensor updates from a microcontroller or sensor gateway, compute risk, store warning and danger events, and display both real-time and historical data in a browser.

Key capabilities:
- Live monitoring dashboard at `/`
- Sensor update API at `/update`
- Live data endpoint at `/get`
- SQLite-backed event store and history endpoint at `/all-data`
- Monthly report endpoint at `/monthly-report`
- Risk scoring that accounts for temperature, gas, change rates, and absence

## End-to-End Flow

1. A sensor system (for example, a microcontroller or IoT gateway) reads real hardware values:
   - Temperature sensor
   - Gas sensor
   - Presence / absence detection
2. The sensor system posts data as JSON to the Flask server at `/update`.
3. The Flask application validates incoming sensor values and computes:
   - temperature risk
   - gas risk
   - rate-of-change risk
   - absence risk
4. The server updates the current live state and returns a computed risk score and status.
5. If the status is `WARNING` or `DANGER`, the server stores the event in the SQLite database.
6. The dashboard at `/` polls `/get` every few seconds to reflect live readings.
7. The dashboard also polls `/all-data` to display recent stored events.
8. Users can review:
   - current kitchen conditions
   - the latest risk score
   - recent warnings and dangers
   - monthly summary data

## Applicability for Electronics and Communication Engineering (ECE)

This project is relevant to ECE students and practitioners because it demonstrates:
- IoT sensor integration with real data sources
- practical use of temperature, gas, and presence sensors
- analog-to-digital conversion and digital signal processing concepts
- threshold-based safety monitoring
- communication between embedded devices and a web server over HTTP
- the use of lightweight APIs for transmitting sensor telemetry

ECE learners can extend the project by connecting actual hardware, calibrating sensor readings, and adding new sensor inputs such as flame or smoke detection.

## Applicability for Computer Science Engineering (CSE)

This project is also useful for CSE students because it covers:
- web application architecture with Flask
- RESTful API design
- JSON-based communication
- state management for live sensor dashboards
- persistent storage using SQLite
- client-side polling and dynamic UI updates
- risk evaluation logic and application-level decision-making

CSE learners can build on the project by improving front-end visualization, adding authentication, or integrating machine learning models for predictive safety analysis.

## Features

- Live risk scoring based on temperature, gas, rate of change, and absence
- Real-time UI updates every 2 seconds via `/get`
- Database-backed event storage for warning/danger conditions
- Dashboard history panel showing recent stored events from `/all-data`
- Alert overlay for elevated risk conditions
- Monthly summary reporting via `/monthly-report`
- Clear separation between live state and historical event data

## Files

- `server.py` — main Flask application with database integration
- `app.py` — earlier version of the Flask dashboard without SQLite storage
- `templates/index.html` — front-end dashboard template updated to also load database history

## How it works

1. Sensor data is posted to `/update` as JSON.
2. The server computes a risk score and status.
3. If the status is `WARNING` or `DANGER`, the event is stored in SQLite.
4. The dashboard polls `/get` to update live values.
5. The dashboard also polls `/all-data` to display recent saved events.
6. The dashboard can be used for monitoring and quick incident review.

## Run locally

1. Install Python 3.10+.
2. Install Flask if it is not already installed:

```bash
python -m pip install Flask
```

3. Start the server:

```bash
python server.py
```

4. Open the dashboard in a browser:

```text
http://127.0.0.1:5000/
```

## API Endpoints

- `GET /` — render the dashboard
- `POST /update` — accept live sensor JSON updates
- `GET /get` — return the latest live state
- `GET /all-data` — return stored risk events from the SQLite database
- `GET /monthly-report` — return events for the current month

## Database

The application uses `kitchen_safety.db` and stores events in the `risk_events` table. Stored fields include:

- `timestamp`
- `temp`
- `gas`
- `person`
- `temp_rate`
- `gas_rate`
- `absence_time`
- `risk_score`
- `status`

The database enables historical analysis and reporting of kitchen safety events.

## Future improvements

- Add authentication for API endpoints
- Add visualization charts for temperature and gas trends
- Add configurable thresholds in the dashboard
- Add event filtering and pagination for the database history
- Add support for real hardware sensor input and microcontroller integration
- Add alert notifications via email or messaging for critical events
- Enhance analytics with predictive risk modeling

```json
{
  "fileName": "README.md"
}
```