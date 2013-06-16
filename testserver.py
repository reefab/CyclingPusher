from flask import Flask, request
app = Flask(__name__)

@app.route("/fitnessActivities", methods=["POST"])
def test():
    data = request.stream.read()
    app.logger.debug(data)

    return "ok"

if __name__ == "__main__":
    app.run(host='0.0.0.0', debug=True)
