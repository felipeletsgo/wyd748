# syntax=docker/dockerfile:1

FROM golang:1.26.5-alpine AS build

WORKDIR /src

COPY wydgo748/go.mod wydgo748/go.sum ./
RUN go mod download

COPY wydgo748/ ./
RUN CGO_ENABLED=0 GOOS=linux go build -trimpath -ldflags="-s -w" -o /out/tm ./cmd/server

FROM node:22-alpine AS web

WORKDIR /web

COPY wydgo748/web/portal/package.json wydgo748/web/portal/package-lock.json ./
RUN npm ci

COPY wydgo748/web/portal/ ./
RUN npm run build

FROM alpine:3.23

RUN apk add --no-cache ca-certificates tzdata \
    && addgroup -S wydgo \
    && adduser -S -G wydgo wydgo

WORKDIR /app

COPY --from=build /out/tm /app/tm
COPY --chown=wydgo:wydgo wydgo748/data/ /app/data/
COPY --from=web --chown=wydgo:wydgo /web/dist/ /app/web/portal/dist/

USER wydgo

EXPOSE 8281

ENTRYPOINT ["/app/tm"]
