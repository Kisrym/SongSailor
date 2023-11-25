import requests, sys
from json import load

class Refresh:
    def __init__(self):
        with open(r"src/config.json") as f:
            data = load(f)
            self.refresh_token = data["refresh_token"]
            self.auth = data["auth"]

    def refresh(self):
        r = requests.post(r"https://accounts.spotify.com/api/token",
                        data = {"grant_type" : "refresh_token", "refresh_token" : self.refresh_token},
                        headers = {"Authorization" : "Basic " + self.auth}
                    )

        if len(r.json()) > 2:
            return r.json()["access_token"]
        else:
            sys.exit(2)

if __name__ == "__main__":
    a = Refresh()
    print(a.refresh())
