const path = require('path');
const HtmlWebpackPlugin = require('html-webpack-plugin');

const config = {
	mode: 'development',
	entry: './src/main.ts',
	devtool: 'inline-source-map',
	devServer: {
		contentBase: './public',
		port: '3000',
		inline: 'true',
	},
	module: {
		rules: [
			{
				use: 'ts-loader',
				exclude: /node_modules/,
				test: /\.tsx?$/,
			},
			{
				use: 'babel-loader',
				exclude: /node_modules/,
				test: /\.js$/,
			},
			{
				use: ['style-loader', 'css-loader'],
				exclude: /node_modules/,
				test: /\.css$/,
			},
			{
				use: 'file-loader',
				exclude: /node_modules/,
				test: /\.(png|jpe?g|gif|svg)$/,
			},
		],
	},
	resolve: {
		extensions: ['.ts', '.js'],
	},
	output: {
		filename: 'bundle.js',
		path: path.resolve(__dirname, 'dist'),
	},
	plugins: [
		new HtmlWebpackPlugin({
			template: './src/index.html',
		}),
	],
};
module.exports = config;
