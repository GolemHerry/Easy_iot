FROM python:3.7-alpine

LABEL maintainer="Golem" 

COPY . /app 
WORKDIR /app

RUN pip install -r /requirements.txt 

CMD ["python3", "-u", "main.py"] 
