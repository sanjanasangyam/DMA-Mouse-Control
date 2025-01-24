const express = require('express');
const bodyParser = require('body-parser');
const mongoose = require('mongoose');
const feedback= require('./models/Feedback');

const app=express();
const port=3000;

mongoose.connect('mongodb://localhost:27017/coderone_feedback',{
    useNewUrlParser: true,
    useUnifiedTopology: true
})
.then(()=>console.log('MonngoDB Connected'))
.catch(err=>console.error('MongoDB connection error',err));

app.use(bodyParser.urlencoded({extended:true}));
app.use(express.static('views'));

app.get('/',(req,res)=>{
    res.sendFile(__dirname+'/views/index.html');
});

app.post('/submit-feedback', async(req,res)=>{
    const { name, contactNumber, email, feedback: feedbackText} =req.body;
   
    const feedback=new Feedback({
    name,
    contactNumber,
    email,
    feedback: feedbackText
   }) ;
   try{
    await feedback.save();
    console.log('Feedback saved successfully');
    res.send(`
        <html>
        <head>
        <title>Feedback Submitted</title>
        </head>
        <body>
        <h1>Thank You!</h1>
        <p>Your Feedback has been successfully submittes.</p>
        <a href="/">Go Back to Form</a>
        </body>
        </html>`);
   } catch(err){
      console.error('error Saving feedback:',err);
      res.status(500).send('there was an error in submitting your feedback.');
   }
});

app.listen(port,()=>
{
   console.log('Server is running on http://localhost:${port}') ;
})
