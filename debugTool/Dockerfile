FROM node
LABEL maintainer="Golem" \
	description="MQTT debug tool"

COPY mqttdebug /home/node/mqttdebug

WORKDIR /home/node/mqttdebug

RUN cd /home/node/mqttdebug &&\
	npm run install-all &&\
	npm run build-ui 

CMD ["npm","run","start"]
