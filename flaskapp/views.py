from flaskapp import app
from flask import render_template
import sqlite3

@app.route('/')
def page():
    conn=sqlite3.connect('flask.db')
    cursor=conn.cursor()
    cursor.execute('select * from flask;')
    rows=[row for row in cursor]
    sensor1=[{'time':row[1]} for row in rows if row[0]==1]
    sensor2=[{'time':row[1]} for row in rows if row[0]==2]
    all=[{'sensor':row[0],'time':row[1]} for row in rows]
    conn.close()
    return render_template('index.html',sensor1=sensor1,sensor2=sensor2,all=all)