<?php

// for extra security store the settings in a *.ini file and parse it
function connect($file = 'database.ini') {
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
    } catch (PDOException $e) {
        echo 'Failed to connect to database' . "\n";
        $conn = null;
    }
    return $conn;
}

    
    // UserInfo(Username, Password, FirstName, LastName, LastIp, LastLogin, SessionID)
    // FirstName and LastName are not required to be set 
    // LastIp and LastLogin are set at login
    // wlll we be automatically logging users in at account creation?
function create_new_user($conn, $username, $password, $firstname, $lastname) {
    if (!(is_unique($conn, $username))) {
        return FALSE;
    }

    $hash = password_hash($password, PASSWORD_BCRYPT);
    $sql = 'INSERT INTO UserInfo (Username, Password, FirstName, LastName) ' .
               'VALUES (:username, :password, :firstname, :lastname)';
        
    $stmt = $conn->prepare($sql);   
    $stmt->bindParam(':username', $username, PDO::PARAM_STR);
    $stmt->bindParam(':password', $hash, PDO::PARAM_STR);
    is_null($firstname) ? $stmt->bindParam(':firstname', $firstname, PDO::PARAM_STR)
                        : $stmt->bindParam(':firstname', null, PDO::PARAM_NULL);
    is_null($lastname)  ? $stmt->bindParam(':lastname', $lastname, PDO::PARAM_STR)
                        : $stmt->bindParam(':lastname', null, PDO::PARAM_NULL);
    return $stmt->execute();
}


function insert_session_id($conn, $username, $length = 32) {
    $session = generate_random_string($length); 
    $stmt = $conn->prepare('UPDATE UserInfo SET SessionID = :session WHERE Username = :username');
    $stmt->execute(array(':session' => $session, ':username' => $username));
    return password_hash($session, PASSWORD_BCRYPT);
}


function update_after_login($conn, $username, $ip, $login) {
    $stmt = $conn->prepare('UPDATE UserInfo SET LastIp = :ip, LastLogin = :login WHERE Username = :username');
    $stmt->execute(array(':ip' => $ip, ':login' => $login, ':username' => $username));
    return $stmt->execute();
}


function verify_login($conn, $username, $password) {
    $hash = query_password($conn, $username);
    return password_verify($password, $hash['password'])
           &&
           verify_username($conn, $username);
}


function verify_username($conn, $username) {
    $result = query_username($conn, $username);
    return $result['username'] === $username;
}


function verify_session($conn, $username, $from_client) {
    $session = query_session_id($conn, $username);
    return password_verify($session['sessionid'], $from_client);
}


function query_username($conn, $username) {
    $stmt = $conn->prepare('SELECT * FROM UserInfo WHERE Username = :username LIMIT 1');
    $stmt->execute(array('username' => $username));
    return $stmt->fetch();
}


function query_password($conn, $username) {
    $stmt = $conn->prepare('SELECT password FROM UserInfo WHERE Username = :username');
    $stmt->execute(array(':username' => $username));
    return $stmt->fetch();
}


function query_user_id($conn, $username) {
    $stmt = $conn->prepare('SELECT UserID FROM UserInfo WHERE Username = :username');
    $stmt->execute(array(':username' => $username));
    return $stmt->fetch();
}


function query_session_id($conn, $username) {
    $stmt = $conn->prepare('SELECT SessionID FROM UserInfo WHERE Username = :username');
    $stmt->execute(array(':username' => $username));
    return $stmt->fetch();
}


function is_unique($conn, $username) {
    $row = query_username($conn, $username);
    return is_null($row[0]);
} 


// uses a CSPRNG
function generate_random_string($length = 32) {
    $result = "";
    for ($i = 0; $i < $length; $i++) {
        $random = random_int(33, 126);
        $result .= chr($random);
    }
    return $result;
}

?>

<?php

// testing

//$conn= connect();

//createNewUser($conn, 'sean', 'password', $firstname = null, $lastname = null);
/*
if (verifyLogin($conn, 'sean', 'password')) {
    echo 'Logged in' . "\n";
} else {
    echo 'Cannot login' . "\n";
}
*/

/*
$encrypted = insertSessionID($conn, 'sean');
if (verifySession($conn, 'sean', $encrypted)) {
    echo 'valid session id' . "\n";
}
*/
?>
