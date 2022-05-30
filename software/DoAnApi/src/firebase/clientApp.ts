import { cert, initializeApp } from 'firebase-admin/app';
// import firebase from "firebase-admin/database";
import { getFirestore } from 'firebase-admin/firestore';

var serviceAccount = require('./service-account-file.json');

initializeApp({
    credential: cert(serviceAccount),
    databaseURL: "https://doan-f7c91-default-rtdb.europe-west1.firebasedatabase.app/"
});

const firestore = getFirestore();

export { firestore };