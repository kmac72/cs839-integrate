## Messing around with this dataset: https://www.kaggle.com/datasets/maltegrosse/8-m-spotify-tracks-genre-audio-features?select=spotify.sqlite

import sqlite3

import pandas as pd

BPM_FUDGE_FACTOR = 2 
LIMIT = 100
TEST_BPM = 150

def search_database(connection, bpm):
    # Searches the connected database for a specific bpm (float or int is fine)
    query = "SELECT id, tempo FROM audio_features WHERE tempo BETWEEN {} AND {} LIMIT {}".format(
        bpm - BPM_FUDGE_FACTOR,
        bpm + BPM_FUDGE_FACTOR, 
        LIMIT
    )

    try:
        df = pd.read_sql_query(query, connection)
        print(df.head())
        print(len(df.index))
    except: 
        print("Problem querying database...")
        print("Make sure the spotify database is in this directory as spotify.sqlite.")


if __name__ == "__main__":
    connection = sqlite3.connect("spotify.sqlite")
    search_database(connection, TEST_BPM)
    connection.close()