# -*- coding: utf-8 -*-

import requests
import time
import json
import random, hashlib
import Crypto.PublicKey.RSA
import Crypto.Cipher.PKCS1_v1_5
import  base64

def HttpGuestLogin(guest_url):
    try:
        now = int(time.time())
        rand_str = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
        nonce_str = ""
        while True:
            index = random.randint(0, len(rand_str) - 1)
            if len(nonce_str) == 8:
                break
            nonce_str += rand_str[index]

        md5_str = "nonce" + nonce_str + "timestamp" + str(now)
        md5_str = md5_str.lower()
        md5_str = hashlib.md5(bytes(md5_str, encoding="utf-8")).hexdigest()

        url = guest_url
        url += '?signature=' + str(md5_str).upper() + '&timestamp=' + str(now) + '&nonce=' + nonce_str
        resp = requests.post(url)
        resp_data = resp.json()
        resp.close()
        if resp_data["code"] == 0:
            return resp_data["data"], None
        return None, resp_data
    except Exception as e:
        return None, e

def HttpUserLogin(user_url, rsa_pub_key, name, pwd):
    try:
        now = int(time.time())
        rand_str = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
        nonce_str = ""
        while True:
            index = random.randint(0, len(rand_str) - 1)
            if len(nonce_str) == 8:
                break
            nonce_str += rand_str[index]

        md5_str = "nonce" + nonce_str + "timestamp" + str(now)
        md5_str = md5_str.lower()
        md5_str = hashlib.md5(bytes(md5_str, encoding="utf-8")).hexdigest()

        url = user_url
        url += '?signature=' + str(md5_str).upper() + '&timestamp=' + str(now) + '&nonce=' + nonce_str

        pwd_md5_str = hashlib.md5(bytes(pwd, encoding="utf-8")).hexdigest()
        rsa_pub_key = Crypto.PublicKey.RSA.importKey(bytes(rsa_pub_key, encoding="utf-8"))
        rsa_cipher = Crypto.Cipher.PKCS1_v1_5.new(rsa_pub_key)
        pwd = rsa_cipher.encrypt(pwd_md5_str.encode())
        pwd = base64.b64encode(pwd).decode()
        post_json = {
            "loginUser": name,
            "password": pwd,
            "clientVersion": {
                "client": "android",
                "version": "1.0.23"
            },
            "deviceInfo": {
                "IP": "13.24.168.2",
                "IMEI1": "863934031439108"
            }
        }

        resp = requests.post(url, headers={'Content-Type': 'application/json'}, data=json.dumps(post_json))
        resp_data = resp.json()
        resp.close()

        if resp_data["code"] == 0:
            return resp_data["data"], None
        return None, resp_data
    except Exception as e:
        return None, e

