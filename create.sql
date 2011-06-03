CREATE TABLE vessel_pos (
       mmsi INTEGER PRIMARY KEY,
       cog INTEGER,
       sog REAL,
       time_stamp TIMESTAMP WITH TIME ZONE DEFAULT now(),
       nav_status VARCHAR(30) -- convert the code to a string for presentation
       ) WITH OIDS;
SELECT AddGeometryColumn('vessel_pos','pos',4326,'POINT',2);
SELECT AddGeometryColumn('vessel_pos','track',4326,'LINESTRING',2);

-- CREATE INDEX vessel_pos_pkey ON vessel_pos(mmsi); -- created automatically if done through psql
CREATE INDEX vessel_pos_ts_idx ON vessel_pos(time_stamp);

CREATE TABLE vessel_name (
       mmsi INTEGER PRIMARY KEY,
       name VARCHAR(25), -- only need 20
       type_and_cargo INTEGER,
       response_class INTEGER -- 0 do not display differently.  1 == response vessel
);

-- CREATE INDEX vessel_name_pkey ON vessel_name(mmsi); -- created automatically
CREATE INDEX vessel_name_rc_idx ON vessel_name(response_class);

CREATE  TABLE response_class (
        class_id INTEGER PRIMARY KEY NOT NULL UNIQUE,
        class_name VARCHAR(60) NOT NULL UNIQUE
);
INSERT INTO response_class VALUES (0,'Generic Vessel');
INSERT INTO response_class VALUES (1,'Response');
INSERT INTO response_class VALUES (3,'Research');
INSERT INTO response_class VALUES (4,'Skimmer');
INSERT INTO response_class VALUES (5,'Government');

-- Stripped to the minimum for SQLite
-- Assumes 1 receive station or that you do not care
CREATE TABLE pos (
       mmsi INTEGER,
       x REAL,
       y REAL,
       ts INTEGER
);
