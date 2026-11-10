# Online Auction API

Flask and PostgreSQL coursework application with user registration, authentication, auction creation, bidding, messages and auction updates. SQL procedures implement parts of the auction logic.

## Requirements

Docker with Docker Compose. The Python and PostgreSQL images are defined in this directory.

## Run

Copy .env.example to .env and set POSTGRES_PASSWORD and AUCTION_SECRET_KEY to local values. Then run:

```sh
docker compose -f docker-compose-python-psql.yml up --build
```

The API is exposed on localhost port 8080, under /dbproj/. Endpoint handlers are defined in python/app/bd.py.

## Project notes

Historical sample accounts and auctions were removed from the portfolio SQL file. Create local test data through the API. This academic implementation requires a separate review before production use.
