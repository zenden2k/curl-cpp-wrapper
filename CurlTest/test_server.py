from flask import Flask
from flask import request
from flask import Response
from flask import jsonify
import hashlib
import json

app = Flask(__name__)

@app.route('/get')
def get():
    return jsonify({'get': 'ok'})

@app.route('/get_hello')
def get_hello():
    return jsonify({'hello': request.args.get('name')})

@app.route('/get_full')
def get_full():
    return jsonify({
        'first': request.args.get('first'),
        'last': request.args.get('last'),
        'custom_header': request.headers.get('X-Hello-World')
    })

@app.route('/post', methods = ['POST'])
def post():
    return jsonify({'hello': request.form['name']})

@app.route('/upload_multipart', methods = ['POST'])
def upload_multipart():
    file = request.files['file']
    file_hash = hashlib.md5(file.read()).hexdigest() 
    return jsonify({'hash': file_hash, "filename": file.filename, "name": request.form['name']})

@app.route('/upload', methods = ['PUT'])
def upload():
    return jsonify({'hash': hashlib.md5(request.data).hexdigest()}), 201   

@app.route('/empty_post_response', methods = ['POST'])
def empty_post_response():
    return '', 204

@app.route('/put', methods = ['PUT'])
def put():
    data = request.get_json(silent=True)
    return jsonify({'update': data['update']})

@app.route('/delete', methods = ['DELETE'])
def delete():
    return '', 200

@app.route('/trace', methods = ['TRACE'])
def trace():
    return Response(request.data, status=200, mimetype='message/http')

if __name__ == '__main__':
    app.run()
