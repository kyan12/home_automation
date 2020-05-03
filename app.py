"""
Main entry point app into GSLogistics
"""
# pylint: disable=missing-docstring

import os
import sys
import mimetypes
import requests

from requests import Request, Session

from flask import Flask, flash, request, redirect, url_for
from flask import render_template, send_from_directory, jsonify
from flask_mail import Mail, Message
from werkzeug.utils import secure_filename

ROOT_DIRECTORY = os.path.dirname(__file__)
UPLOAD_DIRECTORY = os.path.join(ROOT_DIRECTORY, 'files')
ADMIN_EMAILS = ['kevyan1998@gmail.com']
LIVE_EMAILS = ['kevyan1998@gmail.com']


ESP8266_IP = 'http://192.168.1.169'

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_DIRECTORY
app.config['MAX_CONTENT_LENGTH'] = 64 * 1024 * 1024
app.config['MAIL_SERVER'] = 'smtp.gmail.com'
app.config['MAIL_PORT'] = 465
app.config['MAIL_USERNAME'] = 'homeautomator@gmail.com'
app.config['MAIL_PASSWORD'] = 'Sushi1029'
app.config['MAIL_USE_TLS'] = False
app.config['MAIL_USE_SSL'] = True
app.config['MAIL_DEFAULT_SENDER'] = 'homeautomator@gmail.com'

mail = Mail(app)

@app.route('/', methods=['GET', 'POST'])
def dashboard():
    author = 'IoT Hydroponics'
    led_status = 'off'
    pump_status = 'off'
    temp = 0
    hum = 0
    ultra = 0
    # A button was pressed
    if request.method == 'POST':
        if 'LED' in request.form:
            if request.form['LED'] == 'Turn On':
                print('led on')
                resp = requests.get(url=f'{ESP8266_IP}/led_on')
                print(resp)
                led_status = 'on'
                print('Turn on LED')
            elif request.form['LED'] == 'Turn Off':
                print('led off')
                resp = requests.get(url=f'{ESP8266_IP}/led_off')
                led_status = 'off'
                print('Turn off LED')
        if 'pump' in request.form:
            if request.form['pump'] == 'Turn On':
                print('pump on')
                resp = requests.get(url=f'{ ESP8266_IP }/pump_on')
                print(resp)
                pump_status = 'on'
                print('Turn on pump')
            elif request.form['pump'] == 'Turn Off':
                print('pump off')
                resp = requests.get(url=f'{ ESP8266_IP }/pump_off')
                pump_status = 'off'
                print('Turn off pump')
        if 'sensors' in request.form:
            if request.form['sensors'] == 'REFRESH':
                resp = requests.get(url=f'{ ESP8266_IP }/sensors')
                temp_hum_ultra = resp.headers['data'].split(',')
                temp = temp_hum_ultra[0]
                hum = temp_hum_ultra[1]
                ultra = temp_hum_ultra[2]
                print('Readings refreshed')
    return render_template(
        'dashboard.html',
        author=author,
        led=led_status,
        pump=pump_status,
        temperature=temp,
        humidity=hum,
        height=ultra
    )

def main():
    app.run(host='0.0.0.0', debug=('debug' in sys.argv))


# host='0.0.0.0', port=80,
if __name__ == '__main__':
    main()
