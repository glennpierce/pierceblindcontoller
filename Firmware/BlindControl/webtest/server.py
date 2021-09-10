import os
from flask import Flask
from flask import render_template
from flask_socketio import SocketIO,send, emit

# FLASK_APP=server.py FLASK_ENV=development flask run

template_dir = os.path.abspath('../data')
app = Flask(__name__, template_folder=template_dir)
socketio = SocketIO(app, cors_allowed_origins="*")

@app.route("/")
def index():
    return render_template("index.html")

@app.route("/index.html")
def index2():
    return render_template("index.html")

@app.route("/index.css")
def index_css():
    return render_template("index.css")

@app.route("/index.js")
def index_js():
    return render_template("index.js")

@app.route("/settings")
def settings():
    doc = {}
    doc["firmware"] = 1.2
    doc["opentime"] = 10000
    doc["closetime"] = 8000
    doc["devicename"] = "TestDevice"
    doc["buttons_switched"] = True
    doc["speed"] = 76
    return doc

# @socketio.on('message', namespace='/ws')
@socketio.on('message')
def handle_message(message):
    send(message)

# @socketio.on('json', namespace='/ws')
@socketio.on('json')
def handle_json(json):
    send(json, json=True)

# @socketio.on('my event', namespace='/ws')
@socketio.on('my event')
def handle_my_custom_event(json):
    emit('my response', json)

# @socketio.on('connect', namespace='/ws')
@socketio.on('connect')
def test_connect(auth):
    print("connect attempt")
    emit('my response', {'data': 'Connected'})

# @socketio.on('disconnect', namespace='/ws')
@socketio.on('disconnect')
def test_disconnect():
    print('Client disconnected')

if __name__ == "__main__":
    socketio.run(app, debug=True, host='localhost', port='5000')