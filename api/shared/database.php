<?php

// query functions will return null if no value is found or an exception is thrown
// uniqueness test functions will return false on failures and exceptions
// create functions will return false on failures and exceptions


function connect($file = '../config/database.ini') {
    if (!$settings = parse_ini_file($file, true)) {
        return null;
    }
        

    $dns = $settings['database']['driver'] .
           ':host=' . $settings['database']['host'] .
           ';port=' . $settings['database']['port'] . 
           ';dbname=' . $settings['database']['schema'];
    try {
        $conn = new PDO($dns, $settings['database']['username'],
                        $settings['database']['password']);
        $conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        return $conn;
    } catch (PDOException $e) {
        return null;
    }
}

    
function create_new_user($conn, $username, $password, $firstname, $lastname) {
    if (!(is_unique_name($conn, $username))) {
        return FALSE;
    }

    $hash = password_hash($password, PASSWORD_BCRYPT);
    $sql = 'INSERT INTO userinfo (username, password, firstname, lastname) ' .
               'VALUES (:username, :password, :firstname, :lastname)';
        
    try {
        $stmt = $conn->prepare($sql);   
        $stmt->bindParam(':username', $username, PDO::PARAM_STR);
        $stmt->bindParam(':password', $hash, PDO::PARAM_STR);
        isset($firstname) ? $stmt->bindParam(':firstname', $firstname, PDO::PARAM_STR)
                          : $stmt->bindValue(':firstname', null, PDO::PARAM_NULL);
        isset($lastname)  ? $stmt->bindParam(':lastname', $lastname, PDO::PARAM_STR)
                          : $stmt->bindValue(':lastname', null, PDO::PARAM_NULL);
        return $stmt->execute();
    } catch (PDOException $e) {
        return FALSE;
    }
}


function update_after_login($conn, $id, $ip, $login) {
    try {
        $stmt = $conn->prepare('UPDATE userinfo SET lastip = :ip, lastlogin = :login WHERE userid = :id');
        $stmt->bindParam(':id', $id, PDO::PARAM_INT);
        $stmt->bindParam(':ip', $ip, PDO::PARAM_STR);
        $stmt->bindParam(':login', $login, PDO::PARAM_STR);
        return $stmt->execute();
    } catch (PDOException $e) {
        return FALSE;
    }
}


function verify_login($conn, $username, $password) {
    $id = query_user_id($conn, $username);
    $hash = query_password($conn, $id);
    if (is_null($id) || is_null($hash)) {
        return FALSE;
    }

    return password_verify($password, $hash)
           &&
           verify_username($conn, $id, $username);
}


function verify_username($conn, $id, $from_client) {
    $username = query_username($conn, $id);
    if (is_null($username)) {
        return FALSE;
    }
    return $username === $from_client;
}


function verify_session($conn, $id, $from_client) {
    $session = query_session_id($conn, $id);
    if (is_null($session)) {
        return FALSE;
    }
    return $session === $from_client;
}


function query_username($conn, $id) {
    try {
        $stmt = $conn->prepare('SELECT username FROM userinfo WHERE userid = :id LIMIT 1');
        $stmt->bindParam(':id', $id, PDO::PARAM_INT); 
        $stmt->execute();
        $row = $stmt->fetch(PDO::FETCH_ASSOC);
        return ($row) ? $row['username'] : null; 
    } catch (PDOException $e) {
        return null;
    }
}


function query_password($conn, $id) {
    try {
        $stmt = $conn->prepare('SELECT password FROM userinfo WHERE userid = :id');
        $stmt->bindParam(':id', $id, PDO::PARAM_INT); 
        $stmt->execute();
        $row = $stmt->fetch(PDO::FETCH_ASSOC);
        return ($row) ? $row['password'] : null; 
    } catch (PODException $e) {
        return null;
    }
}


function query_session_id($conn, $id) {
    try {
        $stmt = $conn->prepare('SELECT sessionid FROM userinfo WHERE userid = :id');
        $stmt->bindParam(':id', $id, PDO::PARAM_INT); 
        $stmt->execute();
        $row = $stmt->fetch(PDO::FETCH_ASSOC);
        return ($row) ? $row['sessionid'] : null; 
    } catch (PDOException $e) {
        return null;
    }
}


function query_user_id($conn, $username) {
    try {
        $stmt = $conn->prepare('SELECT userid FROM userinfo WHERE username = :username');
        $stmt->bindParam(':username', $username, PDO::PARAM_STR);
        $stmt->execute();
        $row = $stmt->fetch(PDO::FETCH_ASSOC);
        return ($row) ? $row['userid'] : null;    
    } catch (PDOException $e) {
        return null;
    }
}


function query_videos($conn, $id) {
    try {
        $stmt = $conn->prepare('SELECT videoid, title, uploaddate FROM video WHERE userid = :id');
        $stmt->bindParam(':id', $id, PDO::PARAM_INT);
        $stmt->execute();
        $results = $stmt->fetchAll(PDO::FETCH_ASSOC);
        return ($results) ? $results : null;
    } catch (PDOException $e) {
        return null;
    }
}


function query_videopaths($conn, $id) {
    try {
        $stmt = $conn->prepare('SELECT videopath FROM video WHERE userid = :id');
        $stmt->bindParam(':id', $id, PDO::PARAM_INT);
        $stmt->execute();
        $results = $stmt->fetchAll(PDO::FETCH_ASSOC);
        return ($results) ? $results : null;
    } catch (PDOException $e) {
        return null;
    }
}


function is_unique_name($conn, $username) {
    try {
        $stmt = $conn->prepare('SELECT username FROM userinfo WHERE username = :username');
        $stmt->execute(array(':username' => $username));
        $row = $stmt->fetch(PDO::FETCH_ASSOC);
        // empty array means unique
        return ($row) ? FALSE : TRUE;        
    } catch (PDOException $e) {
        return FALSE;
    }
} 


function is_unique_session($conn, $session) {
    try {
        $stmt = $conn->prepare('SELECT sessionid FROM userinfo WHERE sessionid = :session');
        $stmt->execute(array(':session' => $session));
        $row = $stmt->fetch(PDO::FETCH_ASSOC);
        return ($row) ? FALSE : TRUE;
    } catch (PDOException $e) {
        return FALSE;
    }
}


function insert_session($conn, $id, $session) {
    try {
        $stmt = $conn->prepare('UPDATE userinfo SET sessionid = :session WHERE userid = :id');
        $stmt->bindParam(':id', $id, PDO::PARAM_INT);
        $stmt->bindParam('session', $session, PDO::PARAM_STR);
        return $stmt->execute();
    } catch (PDOException $e) {
        return FALSE;
    }
}


function insert_video($conn, $id, $title, $upload_date, $num_frames,
                      $fps, $width, $height, $video_path) {
    try {
        $stmt = $conn->prepare('INSERT INTO video (userid, title, uploaddate, numframes, framespersecond, width, height, videopath) VALUES (:id, :title, :uploaddate, :numframes, :framespersecond, :width, :height, :videopath)');
        $stmt->bindParam(':id', $id, PDO::PARAM_INT);
        $stmt->bindParam(':title', $title, PDO::PARAM_STR);
        $stmt->bindParam(':uploaddate', $upload_date, PDO::PARAM_STR);
        $stmt->bindParam(':numframes', $num_frames, PDO::PARAM_INT);
        $stmt->bindParam(':framespersecond', $fps, PDO::PARAM_INT);
        $stmt->bindParam(':width', $width, PDO::PARAM_INT);
        $stmt->bindParam(':height', $height, PDO::PARAM_INT);
        $stmt->bindParam(':videopath', $video_path, PDO::PARAM_STR);
        return $stmt->execute();
    } catch (PDOException $e) {
        return FALSE;
    }
}


?>
