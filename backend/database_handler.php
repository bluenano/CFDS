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

	
	// UserInfo(Username, Password, FirstName, LastName, LastIp, LastLogin)
	// FirstName and LastName are not required to be set 
	// LastIp and LastLogin are set at login
	// wlll we be automatically logging users in at account creation?
function createNewUser($conn, $username, $password, $firstname, $lastname) {
	if (!(isUnique($conn, $username))) {
		return FALSE;
	}

	$hash = password_hash($password, PASSWORD_DEFAULT);
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


function verifyLogin($conn, $username, $password) {
	$hash = queryPassword($conn, $username);
	return password_verify($password, $hash['password'])
		   &&
	       verifyUsername($conn, $username);
}


function verifyUsername($conn, $username) {
	$result = queryUsername($conn, $username);
	return $result['username'] === $username;
}


function queryUsername($conn, $username) {
	$stmt = $conn->prepare('SELECT * FROM UserInfo WHERE Username = :username LIMIT 1');
	$stmt->execute(array('username' => $username));
	return $stmt->fetch();
}


function isUnique($conn, $username) {
	$row = queryUsername($conn, $username);
	return is_null($row[0]);
} 


function queryPassword($conn, $username) {
	$stmt = $conn->prepare('SELECT password FROM UserInfo WHERE Username = :username');
	$stmt->execute(array(':username' => $username));
	return $stmt->fetch();
}

?>


<?php

// testing
/*
$conn= connect();
if (is_null($conn)) {
	echo 'failed to connect' . "\n";
	exit;
}
createNewUser($conn, 'sean', 'password', $firstname = null, $lastname = null);
if (verifyLogin($conn, 'sean', 'password')) {
	echo 'Logged in' . "\n";
} else {
	echo 'Cannot login' . "\n";
}
*/
?>
