
var app = angular.module('myApp', []);


//Upload controller content 
app.controller('uploadCtrl', function($scope, $http) {

});


//End of Upload Controller Content

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
	  		url: "create_account.php",
	  		data: dataArray,
	  		cache: false,
	  		success: function(data) {
	  		console.log(data);
	  		}
	  	});
	}
});




