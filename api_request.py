import requests
import json

r = requests.get('http://192.168.56.101:8080/api')
json_obj = json.loads(r.text)

print(json_obj)
