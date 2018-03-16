
var app = angular.module('myApp', []);

app.controller('signUpCtrl', function($scope, $http) {

	$scope.createAccount = function ()
	{
		var username = $scope.username;
		var email = $scope.email;
		var firstname = $scope.firstname;
		var lastname = $scope.lastname;
		var password = $scope.password1;

		var dataArray = {username, email, firstname, lastname, password};
		    
		$.ajax({
	  		type:"POST",
	  		url: "index.php",
	  		data: dataArray,
	  		cache: false,
	  		success: function(data) {
	  		console.log(data);
	  		}
	  	});

	}

	$scope.display = function ()
	{
		console.log($scope.username);
		console.log($scope.email);
		console.log($scope.password1);
		console.log($scope.password2);

	}

});