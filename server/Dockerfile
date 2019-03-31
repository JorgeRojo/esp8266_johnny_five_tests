FROM node:alpine
RUN mkdir /src

RUN npm install nodemon -g

WORKDIR /src
ADD api/package.json /src/package.json
RUN yarn

ADD api/nodemon.json /src/nodemon.json

EXPOSE 3000
CMD yarn start