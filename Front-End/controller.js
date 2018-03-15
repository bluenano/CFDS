
var app = angular.module('myApp', []);

app.controller('signUpCtrl', function($scope, $http) {

	$scope.createAccount = function ()
	{
		//Insert server with url
		$http.post("url/server.php", {'username': $scope.username, 'email': $scope.email, 'password': $scope.password1});    
	}

	$scope.display = function ()
	{
		console.log($scope.username);
		console.log($scope.email);
		console.log($scope.password1);
		console.log($scope.password2);

	}

});